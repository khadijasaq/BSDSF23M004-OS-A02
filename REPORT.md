# 📘 REPORT.md — Programming Assignment 02  
**Topic:** File Metadata and Permission Bits in `ls` Implementation  
**Student Name:** [Your Name]  
**Roll No:** [Your Roll Number]  
**Version:** v1.1.0  

---

## 🧩 Question 1 — Difference between `stat()` and `lstat()`

### 🟢 Explanation:
Both `stat()` and `lstat()` are system calls used to retrieve information about files in Unix/Linux systems.  
They both fill a `struct stat` with details such as file type, permissions, size, and timestamps.  

However, the **key difference** lies in how they handle **symbolic links (shortcuts)**:

| Function | Behavior |
|-----------|-----------|
| `stat()` | Follows the symbolic link and returns information about the **target file** (the file it points to). |
| `lstat()` | Returns information about the **link itself**, not the file it points to. |

### 🟢 Example:
If `/home/kali/file_link` is a symbolic link pointing to `/etc/passwd`:
- `stat("/home/kali/file_link")` → returns info for `/etc/passwd`  
- `lstat("/home/kali/file_link")` → returns info for the **link** (`file_link`)

### 🟢 In the context of the `ls` command:
When implementing the `ls -l` (long listing) feature, **`lstat()`** is **more appropriate** because:
- It allows the program to show **the link’s own permissions, owner, and size**.  
- This helps users differentiate between the **symbolic link** and the **file it points to**, which is exactly how the real `ls` command behaves.

---

## 🧩 Question 2 — Understanding `st_mode` and Bitwise Operators

### 🟢 Explanation:
The `st_mode` field inside `struct stat` is an **integer value** where **specific bits** represent:
- The **file type** (e.g., directory, regular file, symbolic link)
- The **permission bits** (read, write, execute) for owner, group, and others

To read these bits, we use **bitwise operators** like `&` along with predefined macros.

### 🟢 Example Code:
```c
struct stat info;
stat("example.txt", &info);

// Checking file type:
if (S_ISDIR(info.st_mode))
    printf("This is a directory.\n");
else if (S_ISREG(info.st_mode))
    printf("This is a regular file.\n");

// Checking permission bits:
if (info.st_mode & S_IRUSR)
    printf("Owner has read permission.\n");
if (info.st_mode & S_IWUSR)
    printf("Owner has write permission.\n");
if (info.st_mode & S_IXUSR)
    printf("Owner has execute permission.\n");
### Feature 3 — Column Display (v1.2.0)

**Summary**
- Implemented default multi-column output formatted "down then across".
- Program adapts to terminal width using `ioctl(TIOCGWINSZ)`, sorts filenames alphabetically, and computes rows/columns dynamically.

**Implementation notes**
- Gather all filenames first into a dynamically allocated `char **` array (using `readdir()` and `strdup()`).
- Track the length of the longest filename while collecting entries.
- Sort the array with `qsort()` and `strcmp()` (to match default `ls` ordering).
- Detect terminal width via `ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws)`; fall back to 80 columns if unavailable.
- Compute:
  - `col_width = maxlen + spacing`
  - `cols = max(1, term_width / col_width)` (and `cols ≤ nfiles`)
  - `rows = ceil(nfiles / cols) = (nfiles + cols - 1) / cols`
- Print by rows: for each row `r`, print file at index `c * rows + r` for `c` in `[0 .. cols-1]`, padding with `printf("%-*s", col_width, name)` for alignment.

**Tests**
- Resized terminal and ran `./bin/ls` — output adjusted columns correctly.
- Verified `./bin/ls -l` still prints long listing (unchanged).
### Feature 4 — Horizontal Column Display (-x) (v1.3.0)

**Summary**
- Added the `-x` flag for horizontal column display (row-major order).
- Now supports three modes:
  - Default (down-then-across)
  - Long listing (-l)
  - Horizontal (-x)

**Implementation**
- Updated `main()` to parse the `-x` flag.
- Added a new function `print_horizontal_columns()` that:
  - Calculates the maximum filename length.
  - Determines terminal width with `ioctl(TIOCGWINSZ)`.
  - Prints filenames left-to-right, wrapping to the next line when needed.
- Updated `do_ls()` to call the correct function based on the display mode flag.

**Testing**
- `./bin/ls` → down-then-across (Feature 3 behavior).
- `./bin/ls -l` → long listing.
- `./bin/ls -x` → horizontal layout (new feature).
- Terminal resizing changes column count automatically.

---



###Featue 4
### Report Questions

**Q1. Compare the complexity of "down then across" vs. "across" logic.**
The "down then across" logic requires pre-calculation of both **rows** and **columns**, because items are printed by row index and column stride (r + c × rows). The horizontal "across" logic is simpler: it only needs terminal width and the next item’s length to decide when to wrap. Therefore, the vertical version is more complex because it must compute grid positions before printing.

**Q2. How were display modes managed?**
A simple set of flags (`long_format` and `horizontal`) were used to track which display mode was chosen.  
- If `-l` is present → long listing.  
- Else if `-x` is present → horizontal display.  
- Else → default vertical display.  
`do_ls()` checks these flags and calls the appropriate function (`print_file_details`, `print_horizontal_columns`, or `print_in_columns`).
##feature 5
Report Questions

Q1. Why must all entries be read into memory before sorting?
To sort the filenames, the program must have all names accessible at once. qsort() operates on an in-memory array, so the entire directory content must first be read into that array.
A drawback is memory usage — directories containing millions of files could consume large amounts of RAM or cause performance delays, as all names must fit in memory.

Q2. What is the purpose and signature of the comparison function in qsort()?
qsort() is a generic sorting function that accepts const void * pointers to handle any data type.
The comparison function defines how two items should be ordered.
For strings:

int cmp_names(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}


It casts the void * arguments to char **, then uses strcmp() to compare the strings alphabetically.
Returning a negative, zero, or positive value tells qsort() the relative order of the two items.







####feature 6
Report Questions

Q1. How do ANSI escape codes work to produce color in a standard Linux terminal? Show the specific code sequence for printing text in green.

ANSI escape codes are special character sequences interpreted by the terminal to change text color or style.
They start with the ESC character (\033) followed by a bracketed code (e.g., [0;32m for green).

Example:

printf("\033[0;32mHello World!\033[0m\n");


This prints “Hello World!” in green, then resets the color with \033[0m.

Q2. To color an executable file, which bits in st_mode are checked?

To detect executables, the program checks the execute permission bits in the file’s st_mode field:

if (st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
    // file is executable


S_IXUSR: Executable by the owner

S_IXGRP: Executable by the group

S_IXOTH: Executable by others
If any of these bits are set, the file is colored green.


---
####FEATURE 7
Report Questions

Q1. What is a "base case" in recursion, and what is the base case here?

A base case is a condition that stops recursion to prevent infinite calls.
In this program, the base case occurs when do_ls() opens a directory that contains no subdirectories —
no further recursive calls are made, and the function returns.

Q2. Why must we construct a full path (e.g., "parent_dir/subdir") before recursive calls?

Because when we descend into a subdirectory, the working directory of the program doesn’t change.
If we call do_ls("subdir") without including its parent path, it will look for "subdir" in the current working directory instead of inside its parent.
Using the full path (parent/subdir) ensures we access the correct nested directory.

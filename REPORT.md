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

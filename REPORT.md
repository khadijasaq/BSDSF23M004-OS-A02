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

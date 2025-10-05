/*
* Programming Assignment 02: lsv1.0.0
* This is the source file of version 1.0.0
* Read the write-up of the assignment to add the features to this base version
* Usage:
*       $ lsv1.0.0 
*       % lsv1.0.0  /home
*       $ lsv1.0.0  /home/kali/   /etc/
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <pwd.h>   // for getpwuid()
#include <grp.h>   // for getgrgid()
#include <time.h>  // for ctime()
#include <sys/ioctl.h>   // ioctl, struct winsize
#include <sys/types.h>
#include <limits.h>
#include <errno.h>
// --- Function Prototypes ---
void do_ls(const char *dir, int long_format, int horizontal);
void print_file_details(const char *path, const char *name);
void print_in_columns(const char *dir);
int gather_filenames(const char *dir, char ***names_out, int *maxlen_out);
int cmp_names(const void *a, const void *b);
void print_horizontal_columns(const char *dir);



extern int errno;
int cmp_names(const void *a, const void *b) {
    const char * const *sa = a;
    const char * const *sb = b;
    return strcmp(*sa, *sb);}

void print_in_columns(const char *dir)
{
    char **names = NULL;
    int maxlen = 0;
    int n = gather_filenames(dir, &names, &maxlen);
    if (n <= 0) {
        // nothing to print or error handled already
        if (n == 0) {
            // nothing in directory (except maybe . and ..)
        }
        return;
    }

    // sort alphabetically (default ls behavior)
    qsort(names, n, sizeof(char *), cmp_names);

    // get terminal width using ioctl
    struct winsize ws;
    int term_width = 80; // fallback
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
        term_width = ws.ws_col;
    }

    int spacing = 2; // spaces between columns
    int col_width = maxlen + spacing;
    int cols = term_width / col_width;
    if (cols < 1) cols = 1;
    if (cols > n) cols = n;   // never more columns than items
    int rows = (n + cols - 1) / cols; // ceil(n / cols)

    // Print rows; for each row r, print names[c*rows + r] for c from 0..cols-1
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int idx = c * rows + r;
            if (idx >= n) continue;
            // if not last column, pad to col_width, else print name only
            if (c < cols - 1)
                printf("%-*s", col_width, names[idx]);
            else
                printf("%s", names[idx]);
        }
        printf("\n");
    }

    // free names
    for (int i = 0; i < n; ++i) free(names[i]);
    free(names);
}

void print_file_details(const char *path, const char *filename)
{
    struct stat info;
    struct passwd *pw;   // to get username
    struct group  *gr;   // to get group name
    char timebuf[100];   // for storing time as text

    // Step 1: Get file info
    if (stat(path, &info) == -1)
    {
        perror(path);
        return;
    }

    // Step 2: Print file type and permissions
    // first character: file type
    if (S_ISDIR(info.st_mode))
        printf("d");
    else
        printf("-");

    // owner permissions
    printf((info.st_mode & S_IRUSR) ? "r" : "-");
    printf((info.st_mode & S_IWUSR) ? "w" : "-");
    printf((info.st_mode & S_IXUSR) ? "x" : "-");

    // group permissions
    printf((info.st_mode & S_IRGRP) ? "r" : "-");
    printf((info.st_mode & S_IWGRP) ? "w" : "-");
    printf((info.st_mode & S_IXGRP) ? "x" : "-");

    // others permissions
    printf((info.st_mode & S_IROTH) ? "r" : "-");
    printf((info.st_mode & S_IWOTH) ? "w" : "-");
    printf((info.st_mode & S_IXOTH) ? "x" : "-");

    // Step 3: Number of links
    printf(" %2ld", info.st_nlink);

    // Step 4: Username and group name
    pw = getpwuid(info.st_uid);
    gr = getgrgid(info.st_gid);
    printf(" %-8s %-8s", pw->pw_name, gr->gr_name);

    // Step 5: File size
    printf(" %8ld", info.st_size);

    // Step 6: Last modification time
    // ctime() returns a string with a newline, so remove it
    strncpy(timebuf, ctime(&info.st_mtime), sizeof(timebuf));
    timebuf[strlen(timebuf) - 1] = '\0';
    printf(" %s", timebuf);

    // Step 7: File name
    printf(" %s\n", filename);
}


int main(int argc, char const *argv[])
{
    int long_format = 0;
    int horizontal = 0;
    int start_index = 1;

    // Parse command-line options
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-l") == 0)
        {
            long_format = 1;
            start_index = i + 1;
        }
        else if (strcmp(argv[i], "-x") == 0)
        {
            horizontal = 1;
            start_index = i + 1;
        }
        else
        {
            break;  // stop parsing when we hit a directory name
        }
    }

    // Handle cases with or without directory arguments
    if (argc == 1 || start_index == argc)
    {
        do_ls(".", long_format, horizontal);
    }
    else
    {
        for (int i = start_index; i < argc; i++)
        {
            printf("\n%s:\n", argv[i]);
            do_ls(argv[i], long_format, horizontal);
        }
    }

    return 0;
}



void do_ls(const char *dir, int long_format, int horizontal)
{
    DIR *dp;
    struct dirent *entry;
    char path[1024];

    dp = opendir(dir);
    if (dp == NULL)
    {
        perror(dir);
        return;
    }

    if (long_format)
    {
        // --- Gather all file names first ---
        char **names = NULL;
        int count = 0, capacity = 0;

        while ((entry = readdir(dp)) != NULL)
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            if (count >= capacity)
            {
                capacity = (capacity == 0) ? 64 : capacity * 2;
                names = realloc(names, capacity * sizeof(char *));
            }
            names[count++] = strdup(entry->d_name);
        }
        closedir(dp);

        // --- Sort alphabetically using qsort() ---
        qsort(names, count, sizeof(char *), cmp_names);

        // --- Print in long listing format ---
        for (int i = 0; i < count; i++)
        {
            snprintf(path, sizeof(path), "%s/%s", dir, names[i]);
            print_file_details(path, names[i]);
        }

        // --- Free memory ---
        for (int i = 0; i < count; i++)
            free(names[i]);
        free(names);
    }
    else if (horizontal)
    {
        closedir(dp);
        print_horizontal_columns(dir);   // horizontal mode
        return;
    }
    else
    {
        closedir(dp);
        print_in_columns(dir);           // default vertical mode
        return;
    }
}


// returns number of files, fills *names_out (caller must free each string and the array),
// and fills *maxlen_out with the longest filename length.
// returns -1 on error
int gather_filenames(const char *dir, char ***names_out, int *maxlen_out) {
    DIR *dp;
    struct dirent *entry;
    char **names = NULL;
    int capacity = 0, count = 0;
    int maxlen = 0;

    dp = opendir(dir);
    if (dp == NULL) {
        perror(dir);
        return -1;
    }

    while ((entry = readdir(dp)) != NULL) {
        // skip "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        int len = strlen(entry->d_name);
        if (len > maxlen) maxlen = len;

        if (count >= capacity) {
            capacity = (capacity == 0) ? 64 : capacity * 2;
            char **tmp = realloc(names, capacity * sizeof(char *));
            if (!tmp) {
                perror("realloc");
                // free already allocated names
                for (int i = 0; i < count; ++i) free(names[i]);
                free(names);
                closedir(dp);
                return -1;
            }
            names = tmp;
        }

        names[count] = strdup(entry->d_name);
        if (!names[count]) {
            perror("strdup");
            for (int i = 0; i < count; ++i) free(names[i]);
            free(names);
            closedir(dp);
            return -1;
        }
        count++;
    }

    closedir(dp);
    *names_out = names;
    *maxlen_out = maxlen;
    return count;
}
#include <sys/ioctl.h>

void print_horizontal_columns(const char *dir)
{
    DIR *dp;
    struct dirent *entry;
    char **names = NULL;
    int capacity = 0, count = 0, maxlen = 0;

    dp = opendir(dir);
    if (dp == NULL)
    {
        perror(dir);
        return;
    }

    // 1️⃣ Gather all filenames
    while ((entry = readdir(dp)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        int len = strlen(entry->d_name);
        if (len > maxlen)
            maxlen = len;

        if (count >= capacity)
        {
            capacity = (capacity == 0) ? 64 : capacity * 2;
            names = realloc(names, capacity * sizeof(char *));
        }
        names[count] = strdup(entry->d_name);
        count++;
    }
    closedir(dp);
    if (count == 0) return;

    // 2️⃣ Sort alphabetically
    qsort(names, count, sizeof(char *), (int (*)(const void *, const void *))strcmp);

    // 3️⃣ Determine terminal width
    struct winsize ws;
    int width = 80;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
        width = ws.ws_col;

    int spacing = 2;
    int col_width = maxlen + spacing;
    int current_width = 0;

    // 4️⃣ Print left-to-right horizontally
    for (int i = 0; i < count; i++)
    {
        int next_width = current_width + col_width;
        if (next_width > width)
        {
            printf("\n");
            current_width = 0;
        }
        printf("%-*s", col_width, names[i]);
        current_width += col_width;
    }
    printf("\n");

    // 5️⃣ Free memory
    for (int i = 0; i < count; i++)
        free(names[i]);
    free(names);
}

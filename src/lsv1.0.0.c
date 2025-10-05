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
void do_ls(const char *dir, int long_format);
void print_file_details(const char *path, const char *name);
void print_in_columns(const char *dir);
int gather_filenames(const char *dir, char ***names_out, int *maxlen_out);
int cmp_names(const void *a, const void *b);



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
void do_ls(const char *dir, int long_format);


int main(int argc, char const *argv[])
{
    int long_format = 0;
    int start_index = 1;

    // check if "-l" was given
    if (argc > 1 && strcmp(argv[1], "-l") == 0)
    {
        long_format = 1;
        start_index = 2;
    }

    if (argc == 1 || (argc == 2 && long_format))
    {
        do_ls(".", long_format);
    }
    else
    {
        for (int i = start_index; i < argc; i++)
        {
            printf("\n%s:\n", argv[i]);
            do_ls(argv[i], long_format);
        }
    }

    return 0;
}

void do_ls(const char *dir, int long_format)
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

    // ✅ If -l option is used → show long listing
    if (long_format)
    {
        while ((entry = readdir(dp)) != NULL)
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);
            print_file_details(path, entry->d_name);
        }
    }
    else
    {
        // ✅ Default mode → multi-column display (Feature 3)
        // We don’t use this dp anymore, because print_in_columns reopens the directory
        closedir(dp);
        print_in_columns(dir);
        return;
    }

    closedir(dp);
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

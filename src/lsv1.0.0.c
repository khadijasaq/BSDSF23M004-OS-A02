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

extern int errno;
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

    while ((entry = readdir(dp)) != NULL)
    {
        // skip "." and ".." if you want
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

        if (long_format)
            print_file_details(path, entry->d_name);
        else
            printf("%s\n", entry->d_name);
    }

    closedir(dp);
}


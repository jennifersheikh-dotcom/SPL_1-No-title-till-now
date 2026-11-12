
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>

#include "assignment.h"
#include "common.h"

static int is_source_file(const char *name)
{
    const char *ext = strrchr(name, '.');

    if (ext == NULL)
        return 0;

    return strcmp(ext, ".c") == 0 ||
           strcmp(ext, ".cpp") == 0 ||
           strcmp(ext, ".java") == 0 ||
           strcmp(ext, ".py") == 0;
}

void assignment_scan_submissions(void)
{
    char path[4096];

    printf("\nEnter submission directory: ");

    if (fgets(path, sizeof(path), stdin) == NULL)
        return;

    string_trim(path);

    if (!directory_exists(path))
    {
        printf("Error: Directory does not exist.\n");
        return;
    }

    DIR *dir = opendir(path);

    if (dir == NULL)
    {
        perror("Unable to open directory");
        return;
    }

    struct dirent *entry;
    int student_count = 0;
    int source_count = 0;

    printf("\n========== SUBMISSION SCANNER ==========\n");

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;

        char full_path[8192];

        int written = snprintf(full_path, sizeof(full_path),
                               "%s/%s", path, entry->d_name);

        if (written < 0 || (size_t)written >= sizeof(full_path))
            continue;

        struct stat info;

        if (stat(full_path, &info) != 0)
            continue;

        if (!S_ISDIR(info.st_mode))
            continue;

        student_count++;

        printf("\nStudent folder: %s\n", entry->d_name);

        DIR *student_dir = opendir(full_path);

        if (student_dir == NULL)
        {
            printf("  Cannot access folder.\n");
            continue;
        }

        struct dirent *file;
        int found = 0;

        while ((file = readdir(student_dir)) != NULL)
        {
            if (is_source_file(file->d_name))
            {
                printf("  Source file: %s\n", file->d_name);
                found++;
                source_count++;
            }
        }

        if (found == 0)
            printf("  No source file found.\n");

        closedir(student_dir);
    }

    closedir(dir);

    printf("\n========== SCAN SUMMARY ==========\n");
    printf("Student folders: %d\n", student_count);
    printf("Source files:    %d\n", source_count);
    printf("==================================\n");
}


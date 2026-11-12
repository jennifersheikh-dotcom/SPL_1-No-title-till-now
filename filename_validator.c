
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "assignment.h"
#include "common.h"

static int valid_submission_name(const char *student_id,
                                 const char *filename)
{
    const char *extensions[] = {
        ".c", ".cpp", ".java", ".py"
    };

    char expected[512];

    for (size_t i = 0; i < 4; i++)
    {
        int n = snprintf(expected, sizeof(expected),
                         "%s%s", student_id, extensions[i]);

        if (n >= 0 && (size_t)n < sizeof(expected) &&
            strcmp(filename, expected) == 0)
            return 1;
    }

    return 0;
}

void assignment_validate_filenames(void)
{
    char root[4096];

    printf("\nEnter submission directory: ");

    if (fgets(root, sizeof(root), stdin) == NULL)
        return;

    string_trim(root);

    if (!directory_exists(root))
    {
        printf("Directory not found.\n");
        return;
    }

    DIR *dir = opendir(root);

    if (dir == NULL)
    {
        perror("Unable to open directory");
        return;
    }

    struct dirent *entry;
    int valid = 0;
    int invalid = 0;

    printf("\n========== FILENAME VALIDATION ==========\n");

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        char folder[8192];

        int n = snprintf(folder, sizeof(folder),
                         "%s/%s", root, entry->d_name);

        if (n < 0 || (size_t)n >= sizeof(folder))
            continue;

        struct stat info;

        if (stat(folder, &info) != 0 || !S_ISDIR(info.st_mode))
            continue;

        DIR *student_dir = opendir(folder);

        if (student_dir == NULL)
            continue;

        struct dirent *file;
        int found_source = 0;

        while ((file = readdir(student_dir)) != NULL)
        {
            const char *ext = strrchr(file->d_name, '.');

            if (ext == NULL)
                continue;

            if (strcmp(ext, ".c") != 0 &&
                strcmp(ext, ".cpp") != 0 &&
                strcmp(ext, ".java") != 0 &&
                strcmp(ext, ".py") != 0)
                continue;

            found_source = 1;

            if (valid_submission_name(entry->d_name,
                                      file->d_name))
            {
                printf("[VALID]   %s/%s\n",
                       entry->d_name, file->d_name);
                valid++;
            }
            else
            {
                printf("[INVALID] %s/%s\n",
                       entry->d_name, file->d_name);
                invalid++;
            }
        }

        if (!found_source)
            printf("[NO SOURCE] %s\n", entry->d_name);

        closedir(student_dir);
    }

    closedir(dir);

    printf("\nValid filenames:   %d\n", valid);
    printf("Invalid filenames: %d\n", invalid);
}

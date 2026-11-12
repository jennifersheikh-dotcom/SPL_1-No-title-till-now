
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

#include "assignment.h"
#include "common.h"

static int has_source_file(const char *folder)
{
    DIR *dir = opendir(folder);

    if (dir == NULL)
        return 0;

    struct dirent *entry;
    int found = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        const char *ext = strrchr(entry->d_name, '.');

        if (ext != NULL &&
            (strcmp(ext, ".c") == 0 ||
             strcmp(ext, ".cpp") == 0 ||
             strcmp(ext, ".java") == 0 ||
             strcmp(ext, ".py") == 0))
        {
            found = 1;
            break;
        }
    }

    closedir(dir);
    return found;
}

void assignment_validate_structure(void)
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
    int passed = 0;
    int failed = 0;

    printf("\n========== STRUCTURE VALIDATION ==========\n");

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

        if (has_source_file(folder))
        {
            printf("[PASS] %s\n", entry->d_name);
            passed++;
        }
        else
        {
            printf("[FAIL] %s: no source file\n", entry->d_name);
            failed++;
        }
    }

    closedir(dir);

    printf("\nPassed: %d\n", passed);
    printf("Failed: %d\n", failed);
}


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>

#include "language_detector.h"
#include "common.h"

static int extension_matches(const char *filename,
                             const char *extension)
{
    const char *dot = strrchr(filename, '.');

    if (dot == NULL)
        return 0;

    dot++;

    while (*dot && *extension)
    {
        if (tolower((unsigned char)*dot) !=
            tolower((unsigned char)*extension))
            return 0;

        dot++;
        extension++;
    }

    return *dot == '\0' && *extension == '\0';
}

Language detect_language(const char *filename)
{
    if (filename == NULL)
        return LANG_UNKNOWN;

    if (extension_matches(filename, "c"))
        return LANG_C;

    if (extension_matches(filename, "cpp") ||
        extension_matches(filename, "cc") ||
        extension_matches(filename, "cxx"))
        return LANG_CPP;

    if (extension_matches(filename, "java"))
        return LANG_JAVA;

    if (extension_matches(filename, "py"))
        return LANG_PYTHON;

    return LANG_UNKNOWN;
}

const char *language_name(Language language)
{
    switch (language)
    {
        case LANG_C:      return "C";
        case LANG_CPP:    return "C++";
        case LANG_JAVA:   return "Java";
        case LANG_PYTHON: return "Python";
        default:          return "Unknown";
    }
}

void assignment_detect_languages(void)
{
    char root[4096];

    printf("\nEnter submission directory: ");

    if (fgets(root, sizeof(root), stdin) == NULL)
        return;

    string_trim(root);

    if (root[0] == '\0')
    {
        printf("Error: Directory path cannot be empty.\n");
        return;
    }

    DIR *root_dir = opendir(root);

    if (root_dir == NULL)
    {
        printf("Error: Cannot open directory: %s\n", root);
        return;
    }

    struct dirent *student_entry;
    int students_found = 0;

    printf("\n========== LANGUAGE DETECTION ==========\n");

    while ((student_entry = readdir(root_dir)) != NULL)
    {
        if (strcmp(student_entry->d_name, ".") == 0 ||
            strcmp(student_entry->d_name, "..") == 0)
            continue;

        char student_path[8192];

        int written = snprintf(
            student_path, sizeof(student_path),
            "%s/%s", root, student_entry->d_name
        );

        if (written < 0 || (size_t)written >= sizeof(student_path))
            continue;

        struct stat student_info;

        if (stat(student_path, &student_info) != 0 ||
            !S_ISDIR(student_info.st_mode))
            continue;

        DIR *student_dir = opendir(student_path);

        if (student_dir == NULL)
            continue;

        students_found++;

        printf("\nStudent: %s\n", student_entry->d_name);

        struct dirent *file_entry;
        int source_found = 0;

        while ((file_entry = readdir(student_dir)) != NULL)
        {
            if (strcmp(file_entry->d_name, ".") == 0 ||
                strcmp(file_entry->d_name, "..") == 0)
                continue;

            char file_path[8192];

            written = snprintf(
                file_path, sizeof(file_path),
                "%s/%s", student_path, file_entry->d_name
            );

            if (written < 0 || (size_t)written >= sizeof(file_path))
                continue;

            struct stat file_info;

            if (stat(file_path, &file_info) != 0 ||
                !S_ISREG(file_info.st_mode))
                continue;

            Language language =
                detect_language(file_entry->d_name);

            if (language != LANG_UNKNOWN)
            {
                printf("  File: %-25s Language: %s\n",
                       file_entry->d_name,
                       language_name(language));

                source_found++;
            }
        }

        if (source_found == 0)
            printf("  No supported source file found.\n");

        closedir(student_dir);
    }

    closedir(root_dir);

    printf("\n----------------------------------------\n");
    printf("Student folders scanned: %d\n", students_found);
    printf("Language detection completed.\n");
}

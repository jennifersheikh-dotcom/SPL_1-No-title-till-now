#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assignment.h"
#include "common.h"

void assignment_export_csv(void)
{
    char filename[512];

    printf("\nCSV output filename (e.g. results.csv): ");

    if (fgets(filename, sizeof(filename), stdin) == NULL)
        return;

    string_trim(filename);

    if (filename[0] == '\0')
    {
        printf("Filename cannot be empty.\n");
        return;
    }

    FILE *file = fopen(filename, "w");

    if (file == NULL)
    {
        perror("Unable to create CSV file");
        return;
    }

    fprintf(file, "Student_ID,Total_Tests,Passed_Tests,");
    fprintf(file, "Failed_Tests,Percentage\n");

    char answer[20];
    int count = 0;

    while (1)
    {
        char id[100];
        int total, passed;

        printf("\nStudent ID (or type done): ");

        if (fgets(id, sizeof(id), stdin) == NULL)
            break;

        string_trim(id);

        if (strcmp(id, "done") == 0)
            break;

        if (id[0] == '\0')
        {
            printf("Student ID cannot be empty.\n");
            continue;
        }

        printf("Total tests: ");

        if (fgets(answer, sizeof(answer), stdin) == NULL)
            break;

        char *end;
        long t = strtol(answer, &end, 10);

        if (end == answer || t <= 0 || t > 100000)
        {
            printf("Invalid total.\n");
            continue;
        }

        total = (int)t;

        printf("Passed tests: ");

        if (fgets(answer, sizeof(answer), stdin) == NULL)
            break;

        long p = strtol(answer, &end, 10);

        if (end == answer || p < 0 || p > total)
        {
            printf("Invalid passed count.\n");
            continue;
        }

        passed = (int)p;

        double percentage = (double)passed / total * 100.0;

        fprintf(file, "\"%s\",%d,%d,%d,%.2f\n",
                id, total, passed, total - passed, percentage);

        count++;
        printf("Record added.\n");
    }

    if (fclose(file) != 0)
    {
        perror("Error closing CSV file");
        return;
    }

    printf("\nCSV export completed.\n");
    printf("Records exported: %d\n", count);
    printf("File: %s\n", filename);
}

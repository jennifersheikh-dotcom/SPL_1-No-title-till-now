
#include <stdio.h>
#include <string.h>
#include "assignment.h"
#include "common.h"

void assignment_compare_outputs(void)
{
    char expected[512];
    char actual[512];

    printf("\nExpected output file: ");

    if (fgets(expected, sizeof(expected), stdin) == NULL)
        return;

    string_trim(expected);

    printf("Actual output file: ");

    if (fgets(actual, sizeof(actual), stdin) == NULL)
        return;

    string_trim(actual);

    FILE *a = fopen(expected, "rb");

    if (a == NULL)
    {
        perror("Cannot open expected output");
        return;
    }

    FILE *b = fopen(actual, "rb");

    if (b == NULL)
    {
        perror("Cannot open actual output");
        fclose(a);
        return;
    }

    int same = 1;
    int ca, cb;

    do
    {
        ca = fgetc(a);
        cb = fgetc(b);

        if (ca != cb)
        {
            same = 0;
            break;
        }
    }
    while (ca != EOF && cb != EOF);

    if (same)
        printf("\n[PASS] Outputs match exactly.\n");
    else
        printf("\n[FAIL] Outputs are different.\n");

    fclose(a);
    fclose(b);
}

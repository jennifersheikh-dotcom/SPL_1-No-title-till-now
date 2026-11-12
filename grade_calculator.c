
#include <stdio.h>
#include <stdlib.h>
#include "assignment.h"

void assignment_calculate_grades(void)
{
    int total;
    int passed;

    printf("\n========== GRADE CALCULATOR ==========\n");

    printf("Total test cases: ");

    if (scanf("%d", &total) != 1 || total <= 0)
    {
        printf("Invalid total.\n");
        return;
    }

    printf("Passed test cases: ");

    if (scanf("%d", &passed) != 1 ||
        passed < 0 || passed > total)
    {
        printf("Invalid passed count.\n");
        return;
    }

    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
        ;

    double percentage = ((double)passed / total) * 100.0;

    printf("\nTotal:      %d\n", total);
    printf("Passed:     %d\n", passed);
    printf("Failed:     %d\n", total - passed);
    printf("Percentage: %.2f%%\n", percentage);

    if (percentage >= 80)
        printf("Grade: A\n");
    else if (percentage >= 70)
        printf("Grade: B\n");
    else if (percentage >= 60)
        printf("Grade: C\n");
    else if (percentage >= 50)
        printf("Grade: D\n");
    else
        printf("Grade: F\n");
}

#include <stdio.h>
#include "assignment.h"

void assignment_run(void) {

    int choice;

    while (1) {

        printf("\n");
        printf("====================================\n");
        printf("       MEOW ASSIGNMENT CHECKER\n");
        printf("====================================\n");

        printf("1. Submission Scanner\n");
        printf("2. Filename Validator\n");
        printf("3. Structure Validator\n");
        printf("4. Test Case Runner\n");
        printf("5. Output Comparator\n");
        printf("6. Grade Calculator\n");
        printf("7. CSV Generator\n");
printf("8. Language Detector\n");
        printf("0. Return to Main Menu\n");

        printf("------------------------------------\n");
        printf("assign> ");

        if (scanf("%d", &choice) != 1) {

            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);

            printf("Invalid input.\n");
            continue;
        }

        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);

        switch (choice) {

            case 1:
                assignment_scan_submissions();
                break;

            case 2:
                assignment_validate_filenames();
                break;

            case 3:
                assignment_validate_structure();
                break;

            case 4:
                assignment_run_tests();
                break;

            case 5:
                assignment_compare_outputs();
                break;

            case 6:
                assignment_calculate_grades();
                break;

            case 7:
                assignment_export_csv();
                break;
		case 8:
    assignment_detect_languages();
    break;
            case 0:
                printf("Returning to MEOW CLI...\n");
                return;

            default:
                printf("Invalid selection.\n");
        }
    }
}






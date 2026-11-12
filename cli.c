
#include <stdio.h>
#include <string.h>

#include "cli.h"
#include "assignment.h"
#include "common.h"

void cli_print_help(void)
{
    printf("\nAvailable commands:\n\n");

    printf("help       List available commands\n");
    printf("version    Display version\n");
    printf("clear      Clear terminal\n");
    printf("explorer   Open file explorer\n");
    printf("assign     Open assignment checker\n");
    printf("compress   Open compressor\n");
    printf("exit       Close MEOW CLI\n\n");
}

void cli_run(void)
{
    char input[256];

    while (1)
    {
        printf("meow> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        string_trim(input);
        string_to_lower(input);

        if (strcmp(input, "help") == 0)
        {
            cli_print_help();
        }
        else if (strcmp(input, "version") == 0)
        {
            printf("MEOW CLI version 0.1.0\n");
        }
        else if (strcmp(input, "clear") == 0)
        {
            terminal_clear();
        }
        else if (strcmp(input, "explorer") == 0)
        {
            printf("File Explorer coming soon.\n");
        }
        else if (strcmp(input, "assign") == 0)
        {
            assignment_run();
        }
        else if (strcmp(input, "compress") == 0)
        {
            printf("Compressor coming soon.\n");
        }
        else if (strcmp(input, "exit") == 0)
        {
            printf("Goodbye!\n");
            break;
        }
        else if (input[0] == '\0')
        {
            continue;
        }
        else
        {
            printf("Unknown command. Type help.\n");
        }
    }
}

#include <stdio.h>
#include "cli.h"

int main(void) {

    printf("\nWELCOME TO MEOW CLI\n\n");

    /* Call your original logo rendering function here. */

    printf("SmartFile can manage your files and process student assignments.\n\n");

    cli_run();

    return 0;
}
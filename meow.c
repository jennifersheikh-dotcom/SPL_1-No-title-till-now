#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "assignment.h"
#include "huffman.h"

#define VERSION "0.1.0"
#define MAX_INPUT 256
#define BLUE 75
#define PURPLE 177
#define SHADOW 240

static const char *LOGO[] = {
    ".............................................#...............#..",
    ".............................................#...............#..",
    "...........................................###.......#.......###",
    ".............................................#......###......#..",
    ".....#.........#........#######.....#####..###...##.....##...###",
    "....###.......###.......#..........#.....#...#...##.....##...#..",
    "....#.#......##.#.......#.........##.....##..##.............##..",
    "....#..#.....#...#......#.........#.......#...#.............#...",
    "...##..##...#....#......#.........#.......#...#.............#...",
    "...#....#...#....##.....#####.....#.......#...#....#####....#...",
    "...#.....###......#.....#.........#.......#...#...#.....#...#...",
    "...#..............#.....#.........#.......#....#..#.....#..#....",
    "...#..............#.....#.........#.......#....#..#.....#..#....",
    "..##..............##....#.........##.....##....#.#.......#.#....",
    "###...##......##...###..#..........#.....#.....###.......###....",
    "..#...##.###..##...#....#######.....#####.......#.........#.....",
    "###.......#........###..........................................",
    "..#................#............................................",
    "..#................#............................................"
};
static const int LOGO_HEIGHT = (int)(sizeof(LOGO) / sizeof(LOGO[0]));

static int use_color = 1;
static int use_shadow = 1;
static int wide_logo = 0;

static int color_for_column(int x) {
    if (x < 23) return PURPLE;
    if (x < 44) return BLUE;
    return PURPLE;
}

static void set_fg(int color) {
    if (use_color) printf("\033[38;5;%dm", color);
}

static void reset_color(void) {
    if (use_color) printf("\033[0m");
}

static void render_logo(void) {
    int width = (int)strlen(LOGO[0]);
    int total_width = width + (use_shadow ? 1 : 0);
    int total_height = LOGO_HEIGHT + (use_shadow ? 1 : 0);

    if (wide_logo) {
        for (int y = 0; y < total_height; ++y) {
            for (int x = 0; x < total_width; ++x) {
                int lit = (y < LOGO_HEIGHT && x < width && LOGO[y][x] == '#');
                int shadow = use_shadow && y > 0 && x > 0 &&
                    (y - 1 < LOGO_HEIGHT) && (x - 1 < width) &&
                    LOGO[y - 1][x - 1] == '#';

                if (lit) {
                    set_fg(color_for_column(x));
                    printf("██");
                    reset_color();
                } else if (shadow) {
                    set_fg(SHADOW);
                    printf("░░");
                    reset_color();
                } else {
                    printf("  ");
                }
            }
            putchar('\n');
        }
        return;
    }

    if (total_height % 2 != 0) ++total_height;

    for (int y = 0; y < total_height; y += 2) {
        for (int x = 0; x < total_width; ++x) {
            int top = -1, bottom = -1;
            int top_shadow = 0, bottom_shadow = 0;

            if (y < LOGO_HEIGHT && x < width && LOGO[y][x] == '#')
                top = color_for_column(x);
            if (y + 1 < LOGO_HEIGHT && x < width && LOGO[y + 1][x] == '#')
                bottom = color_for_column(x);

            if (use_shadow) {
                if (y > 0 && y - 1 < LOGO_HEIGHT && x > 0 && x - 1 < width &&
                    LOGO[y - 1][x - 1] == '#') top_shadow = 1;
                if (y + 1 > 0 && y < LOGO_HEIGHT && x > 0 && x - 1 < width &&
                    LOGO[y][x - 1] == '#') bottom_shadow = 1;
            }

            if (top < 0 && bottom < 0) {
                if (top_shadow || bottom_shadow) {
                    set_fg(SHADOW);
                    printf("░");
                    reset_color();
                } else {
                    putchar(' ');
                }
            } else if (!use_color) {
                if (top >= 0 && bottom >= 0) printf("█");
                else if (top >= 0) printf("▀");
                else printf("▄");
            } else if (top >= 0 && bottom >= 0 && top == bottom) {
                set_fg(top); printf("█"); reset_color();
            } else if (top >= 0 && bottom < 0) {
                set_fg(top); printf("▀"); reset_color();
            } else if (top < 0 && bottom >= 0) {
                set_fg(bottom); printf("▄"); reset_color();
            } else {
                printf("\033[38;5;%dm\033[48;5;%dm▀", top, bottom);
                reset_color();
            }
        }
        putchar('\n');
    }
}

static void print_welcome(void) {
    printf("\n");
    if (use_color) printf("\033[1;97m");
    printf("WELCOME TO MEOW CLI");
    reset_color();
    printf("\n\n");

    render_logo();

    printf("\n");
    if (use_color) printf("\033[38;5;250m");
    printf("MEOW CLI is a terminal-based file management and assignment-processing tool.\n");
    printf("Use simple commands to explore files, validate assignments and manage tasks.\n");
    reset_color();
    printf("\n");
}

static void print_help(void) {
    printf("\nAvailable commands:\n\n");
    printf("  help       List available commands\n");
    printf("  version    Display version\n");
    printf("  clear      Clear terminal\n");
    printf("  explorer   Open file explorer module (placeholder)\n");
    printf("  assign     Open assignment checker\n");
    printf("  compress   Compress and decompress files\n");
    printf("  exit       Close MEOW CLI\n\n");
}

static void clear_terminal(void) {
    /* ANSI clear-screen + move cursor to top-left; works in Ubuntu terminals. */
    printf("\033[2J\033[H");
    fflush(stdout);
}

static void run_placeholder(const char *module_name, const char *description) {
    printf("\n[%s]\n", module_name);
    printf("%s\n", description);
    printf("This module is not implemented yet.\n\n");
}

static void trim(char *text) {
    size_t len = strlen(text);
    while (len > 0 && isspace((unsigned char)text[len - 1])) {
        text[--len] = '\0';
    }

    size_t start = 0;
    while (text[start] && isspace((unsigned char)text[start])) ++start;
    if (start > 0) memmove(text, text + start, strlen(text + start) + 1);
}

static void to_lowercase(char *text) {
    for (size_t i = 0; text[i]; ++i)
        text[i] = (char)tolower((unsigned char)text[i]);
}

static void compressor_menu(void)
{
    char choice[16];
    char input_path[MAX_INPUT];
    char output_path[MAX_INPUT];

    printf("\n");
    printf("====================================\n");
    printf("          MEOW COMPRESSOR           \n");
    printf("====================================\n");
    printf("1. Compress a file\n");
    printf("2. Decompress a file\n");
    printf("3. Back to main menu\n");
    printf("------------------------------------\n");
    printf("Enter your choice: ");

    if (!fgets(choice, sizeof(choice), stdin))
        return;

    trim(choice);

    if (strcmp(choice, "3") == 0)
    {
        printf("Returning to MEOW CLI.\n\n");
        return;
    }

    if (strcmp(choice, "1") != 0 &&
        strcmp(choice, "2") != 0)
    {
        printf("Invalid choice.\n\n");
        return;
    }

    printf("\nEnter input file path: ");

    if (!fgets(input_path, sizeof(input_path), stdin))
        return;

    trim(input_path);

    if (input_path[0] == '\0')
    {
        printf("Input path cannot be empty.\n\n");
        return;
    }

    printf("Enter output file path: ");

    if (!fgets(output_path, sizeof(output_path), stdin))
        return;

    trim(output_path);

    if (output_path[0] == '\0')
    {
        printf("Output path cannot be empty.\n\n");
        return;
    }

    printf("\n");

    if (strcmp(choice, "1") == 0)
    {
        printf("[COMPRESSING]\n");

        huffman_compress_file(input_path, output_path);
    }
    else
    {
        printf("[DECOMPRESSING]\n");

        huffman_decompress_file(input_path, output_path);
    }

    printf("\n");
}


static void command_loop(void)
{
    char input[MAX_INPUT];

    printf("Type 'help' to see the available commands.\n\n");

    while (1)
    {
        printf("meow> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin))
        {
            printf("\nGoodbye!\n");
            break;
        }

        trim(input);
        to_lowercase(input);

        if (input[0] == '\0')
            continue;

        if (strcmp(input, "help") == 0)
        {
            print_help();
        }
        else if (strcmp(input, "version") == 0)
        {
            printf("MEOW CLI version %s\n\n", VERSION);
        }
        else if (strcmp(input, "clear") == 0)
        {
            clear_terminal();
        }
        else if (strcmp(input, "explorer") == 0)
        {
            run_placeholder(
                "FILE EXPLORER",
                "The file explorer will let you inspect and manage directories."
            );
        }
        else if (strcmp(input, "assign") == 0)
        {
            assignment_run();
        }
        else if (strcmp(input, "compress") == 0)
        {
            compressor_menu();
        }
        else if (strcmp(input, "exit") == 0)
        {
            printf("Closing MEOW CLI. Goodbye!\n");
            break;
        }
        else
        {
            printf(
                "Unknown command: '%s'. Type 'help' to see available commands.\n\n",
                input
            );
        }
    }
}


int main(int argc, char *argv[]) {
    int stdout_is_terminal = isatty(STDOUT_FILENO);
    use_color = stdout_is_terminal;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--no-color") == 0) use_color = 0;
        else if (strcmp(argv[i], "--color") == 0) use_color = 1;
        else if (strcmp(argv[i], "--shadow") == 0) use_shadow = 1;
        else if (strcmp(argv[i], "--no-shadow") == 0) use_shadow = 0;
        else if (strcmp(argv[i], "--wide") == 0) wide_logo = 1;
        else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: meow [--wide] [--no-shadow] [--color|--no-color]\n");
            return 0;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            fprintf(stderr, "Use --help to see startup options.\n");
            return 1;
        }
    }

    print_welcome();
    command_loop();
    return 0;
}

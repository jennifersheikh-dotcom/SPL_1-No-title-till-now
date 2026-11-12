
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>
#include <limits.h>

#include "assignment.h"
#include "language_detector.h"

#define BUFFER_SIZE 4096
#define RUN_TIMEOUT 2
#define MAX_TESTS 100

typedef struct
{
    char input[BUFFER_SIZE];
    char expected[BUFFER_SIZE];
} TestCase;

typedef struct
{
    char student_id[256];
    char source[PATH_MAX];
    Language language;
    int passed;
    int failed;
    int status;
} Student;

enum
{
    STATUS_OK = 0,
    STATUS_NO_SOURCE,
    STATUS_MULTIPLE,
    STATUS_COMPILE_ERROR,
    STATUS_UNKNOWN
};

static void trim_newline(char *s)
{
    s[strcspn(s, "\r\n")] = '\0';
}

static void trim_spaces(char *s)
{
    char *start = s;

    while (isspace((unsigned char)*start))
        start++;

    if (start != s)
        memmove(s, start, strlen(start) + 1);

    size_t len = strlen(s);

    while (len > 0 && isspace((unsigned char)s[len - 1]))
        s[--len] = '\0';
}

static int write_text(const char *path, const char *text)
{
    FILE *f = fopen(path, "w");

    if (!f)
        return 0;

    fputs(text, f);
    fputc('\n', f);

    fclose(f);
    return 1;
}

static int run_process(char *const args[],
                       const char *input_file,
                       const char *output_file,
                       int timeout)
{
    pid_t pid = fork();

    if (pid < 0)
        return -1;

    if (pid == 0)
    {
        int in = open(input_file, O_RDONLY);
        int out = open(output_file,
                       O_WRONLY | O_CREAT | O_TRUNC, 0600);

        if (in < 0 || out < 0)
            _exit(127);

        dup2(in, STDIN_FILENO);
        dup2(out, STDOUT_FILENO);
        dup2(out, STDERR_FILENO);

        close(in);
        close(out);

        execvp(args[0], args);
        _exit(127);
    }

    int status;
    time_t start = time(NULL);

    while (1)
    {
        pid_t result = waitpid(pid, &status, WNOHANG);

        if (result == pid)
        {
            if (WIFEXITED(status))
                return WEXITSTATUS(status);

            return -1;
        }

        if (result < 0)
            return -1;

        if (time(NULL) - start >= timeout)
        {
            kill(pid, SIGKILL);
            waitpid(pid, &status, 0);
            return -2;
        }

        struct timespec delay = {0, 10000000};
        nanosleep(&delay, NULL);
    }
}

static int same_output(const char *expected,
                       const char *actual)
{
    FILE *a = fopen(expected, "r");
    FILE *b = fopen(actual, "r");

    if (!a || !b)
    {
        if (a) fclose(a);
        if (b) fclose(b);
        return 0;
    }

    int ca, cb;

    while (1)
    {
        do
        {
            ca = fgetc(a);
        }
        while (ca != EOF && isspace((unsigned char)ca));

        do
        {
            cb = fgetc(b);
        }
        while (cb != EOF && isspace((unsigned char)cb));

        if (ca != cb)
        {
            fclose(a);
            fclose(b);
            return 0;
        }

        if (ca == EOF)
            break;
    }

    fclose(a);
    fclose(b);

    return 1;
}

static int scan_student(const char *root,
                        const char *folder,
                        Student *student)
{
    char folder_path[PATH_MAX];

    int n = snprintf(folder_path, sizeof(folder_path),
                     "%s/%s", root, folder);

    if (n < 0 || n >= (int)sizeof(folder_path))
        return 0;

    struct stat info;

    if (stat(folder_path, &info) != 0 ||
        !S_ISDIR(info.st_mode))
        return 0;

    snprintf(student->student_id,
             sizeof(student->student_id), "%s", folder);

    student->source[0] = '\0';
    student->language = LANG_UNKNOWN;
    student->status = STATUS_NO_SOURCE;
    student->passed = 0;
    student->failed = 0;

    DIR *dir = opendir(folder_path);

    if (!dir)
        return 1;

    struct dirent *entry;
    int source_count = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        if (strstr(entry->d_name, ":Zone.Identifier"))
            continue;

        char file_path[PATH_MAX];

        n = snprintf(file_path, sizeof(file_path),
                     "%s/%s", folder_path, entry->d_name);

        if (n < 0 || n >= (int)sizeof(file_path))
            continue;

        if (stat(file_path, &info) != 0 ||
            !S_ISREG(info.st_mode))
            continue;

        Language lang = detect_language(entry->d_name);

        if (lang == LANG_UNKNOWN)
            continue;

        source_count++;

        if (source_count == 1)
        {
            snprintf(student->source,
                     sizeof(student->source), "%s", file_path);

            student->language = lang;
        }
    }

    closedir(dir);

    if (source_count == 0)
        student->status = STATUS_NO_SOURCE;
    else if (source_count > 1)
        student->status = STATUS_MULTIPLE;
    else
        student->status = STATUS_OK;

    return 1;
}

static int compile_student(Student *student,
                           const char *temp_dir,
                           char *program,
                           char *class_name,
                           const char *log_file,
                           const char *empty_file)
{
    char *args[8];

    if (student->language == LANG_C ||
        student->language == LANG_CPP)
    {
        snprintf(program, PATH_MAX, "%s/program", temp_dir);

        args[0] = student->language == LANG_C ? "gcc" : "g++";
        args[1] = student->source;
        args[2] = student->language == LANG_C
                    ? "-std=c11" : "-std=c++17";
        args[3] = "-o";
        args[4] = program;
        args[5] = NULL;
    }
    else if (student->language == LANG_JAVA)
    {
        char filename[PATH_MAX];

        snprintf(filename, sizeof(filename), "%s",
                 student->source);

        char *base = strrchr(filename, '/');
        base = base ? base + 1 : filename;

        char *dot = strrchr(base, '.');

        if (dot)
            *dot = '\0';

        snprintf(class_name, PATH_MAX, "%s", base);

        args[0] = "javac";
        args[1] = "-d";
        args[2] = (char *)temp_dir;
        args[3] = student->source;
        args[4] = NULL;
    }
    else
    {
        return 1;
    }

    int result = run_process(args, empty_file, log_file, 15);

    return result == 0;
}

static int run_student_test(Student *student,
                            const char *temp_dir,
                            const char *input_file,
                            const char *output_file,
                            const char *expected_file,
                            const char *log_file,
                            const char *empty_file,
                            TestCase tests[],
                            int count)
{
    char program[PATH_MAX] = "";
    char class_name[PATH_MAX] = "";

    if (student->language != LANG_PYTHON)
    {
        if (!compile_student(student, temp_dir,
                             program, class_name,
                             log_file, empty_file))
        {
            student->status = STATUS_COMPILE_ERROR;
            return 0;
        }
    }

    for (int i = 0; i < count; i++)
    {
        write_text(input_file, tests[i].input);
        write_text(expected_file, tests[i].expected);

        char *args[5];

        if (student->language == LANG_C ||
            student->language == LANG_CPP)
        {
            args[0] = program;
            args[1] = NULL;
        }
        else if (student->language == LANG_PYTHON)
        {
            args[0] = "python3";
            args[1] = student->source;
            args[2] = NULL;
        }
        else
        {
            args[0] = "java";
            args[1] = "-cp";
            args[2] = (char *)temp_dir;
            args[3] = class_name;
            args[4] = NULL;
        }

        int result = run_process(args, input_file,
                                 output_file, RUN_TIMEOUT);

        if (result == 0 &&
            same_output(expected_file, output_file))
        {
            student->passed++;
        }
        else
        {
            student->failed++;
        }
    }

    return 1;
}

void assignment_run_tests(void)
{
    char root[PATH_MAX];
    char temp_template[] = "/tmp/meowXXXXXX";
    char temp_dir[PATH_MAX];

    char input_file[PATH_MAX];
    char output_file[PATH_MAX];
    char expected_file[PATH_MAX];
    char log_file[PATH_MAX];
    char empty_file[PATH_MAX];

    TestCase tests[MAX_TESTS];

    printf("\n========== AUTOMATED TEST RUNNER ==========\n");
    printf("Submission directory: ");

    if (!fgets(root, sizeof(root), stdin))
        return;

    trim_newline(root);
    trim_spaces(root);

    struct stat root_info;

    if (stat(root, &root_info) != 0 ||
        !S_ISDIR(root_info.st_mode))
    {
        printf("Invalid submission directory.\n");
        return;
    }

    printf("\nNumber of test cases: ");

    char line[100];

    if (!fgets(line, sizeof(line), stdin))
        return;

    int count = atoi(line);

    if (count <= 0 || count > MAX_TESTS)
    {
        printf("Invalid test case count.\n");
        return;
    }

    for (int i = 0; i < count; i++)
    {
        printf("\n--- Test Case %d ---\n", i + 1);

        printf("Input: ");

        if (!fgets(tests[i].input, BUFFER_SIZE, stdin))
            return;

        trim_newline(tests[i].input);

        printf("Expected output: ");

        if (!fgets(tests[i].expected, BUFFER_SIZE, stdin))
            return;

        trim_newline(tests[i].expected);
    }

    if (!mkdtemp(temp_template))
    {
        printf("Could not create temporary directory.\n");
        return;
    }

    snprintf(temp_dir, sizeof(temp_dir), "%s", temp_template);

    snprintf(input_file, sizeof(input_file),
             "%s/input.txt", temp_dir);

    snprintf(output_file, sizeof(output_file),
             "%s/output.txt", temp_dir);

    snprintf(expected_file, sizeof(expected_file),
             "%s/expected.txt", temp_dir);

    snprintf(log_file, sizeof(log_file),
             "%s/compiler.log", temp_dir);

    snprintf(empty_file, sizeof(empty_file),
             "%s/empty.txt", temp_dir);

    write_text(empty_file, "");

    DIR *dir = opendir(root);

    if (!dir)
    {
        printf("Cannot open submission directory.\n");
        return;
    }

    printf("\n========== SCANNING SUBMISSIONS ==========\n");

    struct dirent *entry;
    int total = 0;
    int passed_students = 0;

    FILE *csv = fopen("meow_results.csv", "w");

    if (csv)
        fprintf(csv, "Student ID,Language,Passed,Failed,Status,Score\n");

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        Student student;

        if (!scan_student(root, entry->d_name, &student))
            continue;

        if (student.status == STATUS_NO_SOURCE &&
            student.student_id[0] == '\0')
            continue;

        total++;

        printf("\nStudent: %s\n", student.student_id);

        if (student.status == STATUS_NO_SOURCE)
        {
            printf("[NO SOURCE FILE]\n");
        }
        else if (student.status == STATUS_MULTIPLE)
        {
            printf("[MULTIPLE SOURCE FILES - SKIPPED]\n");
        }
        else
        {
            printf("Language: %s\n",
                   language_name(student.language));

            char student_temp[PATH_MAX];

            snprintf(student_temp, sizeof(student_temp),
                     "%s/%s", temp_dir, student.student_id);

            mkdir(student_temp, 0700);

            int ok = run_student_test(
                &student, student_temp,
                input_file, output_file,
                expected_file, log_file,
                empty_file, tests, count
            );

            if (!ok)
            {
                printf("[COMPILE ERROR]\n");
            }
            else
            {
                printf("Passed: %d/%d\n",
                       student.passed, count);

                printf("Failed: %d/%d\n",
                       student.failed, count);

                if (student.passed == count)
                    passed_students++;
            }
        }

        const char *status =
            student.status == STATUS_OK ? "CHECKED" :
            student.status == STATUS_NO_SOURCE ? "NO SOURCE" :
            student.status == STATUS_MULTIPLE ? "MULTIPLE FILES" :
            student.status == STATUS_COMPILE_ERROR ? "COMPILE ERROR" :
            "UNKNOWN";

        double score = student.status == STATUS_OK
            ? (double)student.passed / count * 100.0 : 0.0;

        if (csv)
        {
            fprintf(csv, "%s,%s,%d,%d,%s,%.2f\n",
                    student.student_id,
                    language_name(student.language),
                    student.passed,
                    student.failed,
                    status,
                    score);
        }
    }

    closedir(dir);

    if (csv)
        fclose(csv);

    printf("\n========== FINAL SUMMARY ==========\n");
    printf("Students scanned: %d\n", total);
    printf("Perfect scores:   %d\n", passed_students);
    printf("CSV file: meow_results.csv\n");
    printf("Temporary files: %s\n", temp_dir);
}

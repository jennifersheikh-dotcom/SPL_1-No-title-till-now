#ifndef COMMON_H
#define COMMON_H

void string_trim(char *text);
void string_to_lower(char *text);

int file_exists(const char *path);
int directory_exists(const char *path);

void terminal_clear(void);

#endif
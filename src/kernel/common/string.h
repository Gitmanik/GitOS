#pragma once
#include <stddef.h>

size_t strlen(const char* str);
size_t strnlen(const char* str, size_t max_len);

char* itoa(long num, char* str, int base);
char* ksprintf(char* buf, char* fmt, ...);

int is_digit(char c);
int to_numeric_digit(char c);
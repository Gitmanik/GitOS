#pragma once
#include <stddef.h>
#include <stdarg.h>

size_t strlen(const char* str);
size_t strnlen(const char* str, size_t max_len);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, int n);
char* strrev(char *str);
int strncmp(const char* str1, const char* str2, int n);
int strnlen_terminator(const char* str, int max, char terminator);
int istrncmp(const char* str1, const char* str2, int n);
char tolower(char c);


char* itoa(long num, char* str, int base);
char* ksprintf(char* buf, const char* fmt, ...);
char* kvsprintf(char* buf, const char* fmt, va_list args);

int is_digit(char c);
int to_numeric_digit(char c);
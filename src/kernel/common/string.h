#pragma once
#include <stddef.h>

size_t strlen(const char* str);
char* itoa(long num, char* str, int base);
char* ksprintf(char* buf, char* fmt, ...);
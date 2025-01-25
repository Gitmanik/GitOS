//
// Created by Pawel Reich on 1/11/25.
//

#pragma once

#include <stddef.h>
#include <stdarg.h>

size_t strlen(const char* str);
size_t strnlen(const char* str, size_t max_len);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
char* strrev(char *str);
int strncmp(const char* str1, const char* str2, size_t n);
int strcmp(const char* str1, const char* str2);
size_t strnlen_terminator(const char* str, size_t max, char terminator);
int istrncmp(const char* str1, const char* str2, size_t n);
char tolower(char c);


char* itoa(long num, char* str, int base);
char* uitoa(unsigned long num, char* str, int base);

char* sprintf(char* buf, const char* fmt, ...);
char* vsprintf(char* buf, const char* fmt, va_list args);


int is_digit(char c);
int to_numeric_digit(char c);

void* memset(void* ptr, int c, size_t size);
void* memcpy(void* dstptr, const void* srcptr, size_t size);
int memcmp(void* ptr1, void* ptr2, size_t len);
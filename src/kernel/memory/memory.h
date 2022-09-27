#pragma once
#include <stddef.h>

void* memset(void* ptr, int c, size_t size);
void* memcpy(void* restrict dstptr, const void* restrict srcptr, size_t size);
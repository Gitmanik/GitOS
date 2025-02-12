//
// Created by Paweł Reich on 2/12/25.
//
#include "stddef.h"

extern "C" {
#include <misc.h>
#include "string.h"
}

extern "C" int atexit(void (*)())
{
    return 0;
}

void *operator new(size_t size)
{
    return malloc(size);
}

void *operator new[](size_t size)
{
    return malloc(size);
}

void operator delete(void *p)
{
    free(p);
}

void operator delete[](void *p)
{
    free(p);
}
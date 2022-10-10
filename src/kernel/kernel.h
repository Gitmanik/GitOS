#pragma once
#include <stdarg.h>
#include "common/io.h"

#define BochsBreak() outw(0x8A00,0x8A00); outw(0x8A00,0x08AE0);

void kernel_main();
void kernel_halt();
void kernel_panic(char* fmt, ...);

void kprintf(char* fmt, ...);

#define kdebug(fmt, ...) kprintf("%s:%d: ", __FILE__, __LINE__); kprintf(fmt, __VA_ARGS__);

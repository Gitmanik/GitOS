#pragma once
#include <stdarg.h>
#include "common/io.h"

#define DEBUG_MODE

#define BochsBreak() outw(0x8A00,0x8A00); outw(0x8A00,0x08AE0);

#ifdef __cplusplus
extern "C" {
#endif

void kernel_main(uint32_t magic, void *info_ptr);
void kernel_halt();
void kernel_panic(const char* fmt, ...);
void kernel_page();
extern void kernel_registers();
void kprintf(const char* fmt, ...);

extern const uint32_t kernel_start;
extern const uint32_t kernel_end;


#ifdef __cplusplus
}
#endif

#ifdef DEBUG_MODE
#define kdebug(fmt, ...) kprintf("%s:%d: ", __FILE__, __LINE__); kprintf(fmt, __VA_ARGS__); kprintf("\n");
#else
#define kdebug(fmt, ...)
#endif
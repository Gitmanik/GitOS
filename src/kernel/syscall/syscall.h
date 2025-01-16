#pragma once

#include "idt/idt.h"

#define MAX_SYSCALLS 1024

typedef void*(*SYSCALL)(struct interrupt_frame* frame);

extern void syscall_wrapper();
void syscall_init();

enum syscalls
{
    SYSCALL_GETCHAR,
    SYSCALL_PUTCHAR,
    SYSCALL_PUTSTRING,
    SYSCALL_EXECPROCESS,
    SYSCALL_MALLOC,
    SYSCALL_FREE,
};

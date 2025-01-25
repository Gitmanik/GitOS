#pragma once

extern "C" {
#include "idt/idt.h"
}

#define MAX_SYSCALLS 1024

typedef void*(*SYSCALL)(struct interrupt_frame* frame);
extern "C" {
extern void syscall_wrapper();
void* syscall_handler(int syscall_id, struct interrupt_frame* frame);
}
void syscall_init();


enum syscalls
{
    SYSCALL_GETCHAR,
    SYSCALL_PUTCHAR,
    SYSCALL_PUTSTRING,
    SYSCALL_EXECPROCESS,
    SYSCALL_MALLOC,
    SYSCALL_FREE,
    SYSCALL_GET_PROCESS_ARGUMENTS,
    SYSCALL_EXIT,
};

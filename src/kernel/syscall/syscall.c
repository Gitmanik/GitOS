#include "syscall.h"

#include "idt/idt.h"
#include "kernel.h"
#include "task/task.h"

static SYSCALL syscalls[MAX_SYSCALLS];

static void syscall_register(int syscall_id, SYSCALL func)
{
    if (syscall_id < 0 || syscall_id >= MAX_SYSCALLS)
    {
        kernel_panic("Syscall is out of bounds: %d", syscall_id);
    }

    if (syscalls[syscall_id])
    {
        kernel_panic("Syscall already registered: %d", syscall_id);
    }

    syscalls[syscall_id] = func;
}

static void* syscall_handle_command(int syscall_id, struct interrupt_frame* frame)
{
    void* res = 0;

    if (syscall_id < 0 || syscall_id >= MAX_SYSCALLS)
    {
        return 0;
    }

    SYSCALL func = syscalls[syscall_id];
    if (!func)
    {
        return 0;
    }

    res = func(frame);

    return res;
}

void* syscall_handler(int syscall_id, struct interrupt_frame* frame)
{
    void* res = 0;
    kernel_page();
    task_current_save_state(frame);
    res = syscall_handle_command(syscall_id, frame);
    task_page();
    return res;
}

// SYSCALLS

void* sys$blank(struct interrupt_frame* frame)
{
    (void)(frame);
    return 0;
}

void syscall_init()
{
    syscall_register(SYSCALL_SUM, sys$blank);
}

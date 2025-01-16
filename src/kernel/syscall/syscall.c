#include "syscall.h"

#include <common/assert.h>
#include <common/string.h>

#include "memory/heap/kheap.h"
#include "idt/idt.h"
#include "kernel.h"
#include "task/task.h"
#include "task/process.h"
#include "memory/heap/kheap.h"

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

void* sys$putstring(struct interrupt_frame* frame)
{
    (void)(frame);

    char buf[2048] = {0};

    void* str_ptr = task_peek_stack(task_current(), 0);

    task_copy_string_from(task_current(), str_ptr, buf, 2048);

    kprintf(buf);

    return 0;
}

void* sys$getchar(struct interrupt_frame* frame)
{
    (void)(frame);
    char c = process_popkey(task_current()->process);
    return (void*) ((int) c);
}

void* sys$putchar(struct interrupt_frame* frame)
{
    (void)(frame);
    char c = (char) ((int)task_peek_stack(task_current(), 0));
    kprintf("%c", c);
    return 0;
}

void* sys$execprocess(struct interrupt_frame* frame) {
    (void)(frame);
    void* str_ptr = task_peek_stack(task_current(), 0);
    char path[MAX_PATH] = {0};

    task_copy_string_from(task_current(), str_ptr, path, MAX_PATH);

    int res = 0;
    struct process* new_process = kmalloc(sizeof(struct process));
    res = process_load_switch(path, &new_process);

    if (res < 0) {
        kfree(new_process);
        return (void*) res;
    }

    task_switch(new_process->task);
    task_return(&new_process->task->registers);

    return (void*) res;

}

void* sys$get_process_arguments(struct interrupt_frame* frame) {
    (void)(frame);

    int* argc = task_peek_stack(task_current(), 0);
    char*** argv = task_peek_stack(task_current(), 1);

    struct process* process = process_current();

    char** new_argv = process_malloc(process, sizeof(char*) * process->argc);

    for (int i = 0; i < process->argc; i++) {
        new_argv[i] = process_malloc(process, sizeof(char) * strlen(process->argv[i]) + 1);
        strcpy(new_argv[i], process->argv[i]);
    }

    task_page();

    *argc = process->argc;
    *argv = new_argv;

    kernel_page();

    return 0;
}

void* sys$malloc(struct interrupt_frame* frame) {
    (void)(frame);

    struct process* process = process_current();

    int mem_size = *(int*)task_peek_stack(task_current(), 0);

    return process_malloc(process, mem_size);
}

void* sys$free(struct interrupt_frame* frame) {
    (void)(frame);

    struct process* process = process_current();

    void* mem_to_free = task_peek_stack(task_current(), 0);

    process_free(process, mem_to_free);

    return 0;
}

void syscall_init()
{
    syscall_register(SYSCALL_PUTSTRING, sys$putstring);
    syscall_register(SYSCALL_GETCHAR, sys$getchar);
    syscall_register(SYSCALL_PUTCHAR, sys$putchar);
    syscall_register(SYSCALL_EXECPROCESS, sys$execprocess);
    syscall_register(SYSCALL_MALLOC, sys$malloc);
    syscall_register(SYSCALL_FREE, sys$free);
    syscall_register(SYSCALL_GET_PROCESS_ARGUMENTS, sys$get_process_arguments);
}

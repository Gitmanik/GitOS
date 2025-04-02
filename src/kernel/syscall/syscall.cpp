#include "syscall.hpp"

#include <drivers/graphics/vbe/vbe_graphics.hpp>
#include <fs/pipe/PipeFS.hpp>

extern "C" {
#include <fs/file.h>
#include <common/assert.h>
#include <common/string.h>

#include "memory/heap/kheap.h"
#include "idt/idt.h"
#include "kernel.h"
#include "task/task.h"
#include "task/process.h"
#include "memory/heap/kheap.h"
}


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
        kprintf("Task %s tried to call syscall %d but it is not registered!", task_current()->process->filename, syscall_id);
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
    process* new_process = new process;
    res = process_load_switch(path, new_process);

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

    int* argc = (int*) task_peek_stack(task_current(), 0);
    char*** argv = (char***) task_peek_stack(task_current(), 1);

    struct process* process = task_current()->process;

    char** new_argv = (char**) process_malloc(process, sizeof(char*) * process->argc);

    for (int i = 0; i < process->argc; i++) {
        new_argv[i] = (char*) process_malloc(process, sizeof(char) * strlen(process->argv[i]) + 1);
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

    struct process* process = task_current()->process;

    int mem_size = (int)task_peek_stack(task_current(), 0);

    return process_malloc(process, mem_size);
}

void* sys$free(struct interrupt_frame* frame) {
    (void)(frame);

    struct process* process = task_current()->process;

    void* mem_to_free = task_peek_stack(task_current(), 0);

    process_free(process, mem_to_free);

    return 0;
}

void* sys$exit(struct interrupt_frame* frame) {
    (void)(frame);
    int return_code = (int)task_peek_stack(task_current(), 0);
    kprintf("Process %s exited with return code: %d\n", process_current()->filename, return_code);

    process_terminate(process_current());
    task_switch(process_current()->task);
    task_return(&task_current()->registers);
    return 0;
}

struct FramebufferInfo {
    uint32_t* buffer;
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
};

void * sys$get_framebuffer_info(struct interrupt_frame * frame) {
    (void)(frame);

    FramebufferInfo* fb_info = (FramebufferInfo*) task_peek_stack(task_current(), 0);

    auto vbe = static_cast<VBEGraphics*>(VBEGraphics::the());

    auto* framebuffer = (uint32_t*) task_current()->process->framebuffer;
    uint32_t width = vbe->get_width();
    uint32_t height = vbe->get_height();
    uint32_t bpp = vbe->get_bpp();

    task_page();

    fb_info->buffer = framebuffer;
    fb_info->width = width;
    fb_info->height = height;
    fb_info->bpp = bpp;

    kernel_page();

    return 0;
}

void * sys$fopen(struct interrupt_frame * frame) {
    (void)(frame);

    void* path_ptr = task_peek_stack(task_current(), 0);
    void* mode_ptr = task_peek_stack(task_current(), 1);
    char path[MAX_PATH] = {0};
    char mode[MAX_PATH] = {0};

    task_copy_string_from(task_current(), path_ptr, path, MAX_PATH);
    task_copy_string_from(task_current(), mode_ptr, mode, MAX_PATH);

    return (void*) fopen(path, mode);
}

void * sys$fclose(struct interrupt_frame * frame) {
    (void)(frame);

    return (void*) fclose((int) task_peek_stack(task_current(),0));
}

void * sys$fread(struct interrupt_frame * frame) {
    (void)(frame);

    void* buffer = task_peek_stack(task_current(), 0);
    auto size = (uint32_t) task_peek_stack(task_current(), 1);
    auto nmemb = (uint32_t) task_peek_stack(task_current(), 2);
    int fd = (int) task_peek_stack(task_current(), 3);

    return (void*) fread(buffer, size, nmemb, fd);

}

void * sys$fwrite(struct interrupt_frame * frame) {
    (void)(frame);

    void* buffer = task_peek_stack(task_current(), 0);
    auto size = (uint32_t) task_peek_stack(task_current(), 1);
    auto nmemb = (uint32_t) task_peek_stack(task_current(), 2);
    int fd = (int) task_peek_stack(task_current(), 3);

    return (void*) fwrite(buffer, size, nmemb, fd);

}

void * sys$fstat(struct interrupt_frame * frame) {
    (void)(frame);
    int fd = (int) task_peek_stack(task_current(), 0);
    file_stat* task_stat = (file_stat*) task_peek_stack(task_current(), 1);

    file_stat file;

    void* res = (void*) fstat(fd, &file);

    task_page();

    task_stat->filesize = file.filesize;
    task_stat->flags = file.flags;

    kernel_page();


    return res;
}

void * sys$fseek(struct interrupt_frame * frame) {
    (void)(frame);
    int fd = (int) task_peek_stack(task_current(), 0);
    int offset = (int) task_peek_stack(task_current(), 1);
    FILE_SEEK_MODE whence = (int) task_peek_stack(task_current(), 2);

    void* res = (void*) fseek(fd, offset, whence);

    return res;
}

void* sys$open_ipc(struct interrupt_frame * frame) {
    (void)(frame);

    void* filename_ptr = task_peek_stack(task_current(), 0);
    char filename[MAX_PATH] = {0};

    task_copy_string_from(task_current(), filename_ptr, filename, MAX_PATH);

    uint32_t packet_size = (int) task_peek_stack(task_current(), 1);
    uint32_t count = (int) task_peek_stack(task_current(), 2);

    PipeFS* pipe = new PipeFS(packet_size * count);

    char* path = new char[MAX_PATH];
    strcpy(path, "0:/ipc/");
    strcpy(path + strlen(path), filename);

    mount(path, pipe->get_struct(), pipe);

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
    syscall_register(SYSCALL_EXIT, sys$exit);
    syscall_register(SYSCALL_GET_FRAMEBUFFER_INFO, sys$get_framebuffer_info);
    syscall_register(SYSCALL_FOPEN, sys$fopen);
    syscall_register(SYSCALL_FREAD, sys$fread);
    syscall_register(SYSCALL_FWRITE, sys$fwrite);
    syscall_register(SYSCALL_FSTAT, sys$fstat);
    syscall_register(SYSCALL_FSEEK, sys$fseek);
    syscall_register(SYSCALL_FCLOSE, sys$fclose);
    syscall_register(SYSCALL_OPENIPC, sys$open_ipc);
}

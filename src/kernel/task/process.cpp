#include <drivers/graphics/vbe/vbe_graphics.hpp>

extern "C" {
#include "process.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "fs/file.h"
#include "common/status.h"
#include "task.h"
#include "common/string.h"
#include "kernel.h"
}
#include "formats/elf/ELFFile.hpp"

struct process* current_process = 0;

static struct process* processes[MAX_PROCESSES] = {};

struct process* process_current()
{
    return current_process;
}

struct process* process_get(int process_id)
{
    if (process_id < 0 || process_id >= MAX_PROCESSES)
        return 0;

    return processes[process_id];
}

int process_switch(struct process* process)
{
    current_process = process;
    return 0;
}

int elf_load(const char * filename, ELFFile ** elf_file) {
    int res = fopen(filename, "r");
    int fd = 0;
    void* data = nullptr;

    if (res < 0)
        goto out;
    fd = res;

    struct file_stat stat;
    res = fstat(fd, &stat);
    if (res < 0)
        goto out;

    data = kzalloc(stat.filesize);
    res = fread(data, stat.filesize, 1, fd);
    if (res < 0)
        goto out;

    *elf_file = new ELFFile(data, stat.filesize);

    out:
    fclose(res);
    return res;
}

static int process_load_data(const char* filename, struct process* process)
{
    int res = 0;

    ELFFile* elf_file = nullptr;

    res = elf_load(filename, &elf_file);
    if (res < 0)
        return res;

    res = elf_file->parse();

    process->elf = elf_file;
    process->elf_entry = elf_file->get_entry();

    return res;
}

int process_map_memory(struct process* process)
{
    auto* elf_file = static_cast<ELFFile *>(process->elf);

    auto elf_header = elf_file->get_header();

    for (int i = 0; i < elf_header->e_phnum; i++) {
        auto program_header = elf_file->get_program_header(i);

        void* physical_address;

        if (program_header->p_memsz > program_header->p_filesz)
        {
            void* bss = kzalloc(program_header->p_memsz);
            memcpy(bss, reinterpret_cast<void *>(reinterpret_cast<int>(elf_file->get_header()) + program_header->p_offset), program_header->p_filesz);
            process->bss = bss;
            physical_address = bss;
        } else
        {
            physical_address = reinterpret_cast<void *>(reinterpret_cast<int>(elf_file->get_header()) + program_header->p_offset);
        }

        void* physical_end = (void*) ((int)physical_address + program_header->p_memsz);

        int page_flags = PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL;
        if (program_header->p_flags & ELFFile::p_flags::PF_W) {
            page_flags |= PAGING_IS_WRITEABLE;
        }


    int res = paging_map_to(process->task->page_directory,
        paging_align_address_to_lower_page((void*) program_header->p_vaddr),
        paging_align_address_to_lower_page(physical_address),
        paging_align_address(physical_end),
        page_flags);

        if (res < 0)
            return res;
    }

    return paging_map_to(process->task->page_directory, (void*) (PROGRAM_VIRTUAL_STACK_ADDRESS_END), process->stack, paging_align_address((void*) ((uint32_t) process->stack + PROGRAM_VIRTUAL_STACK_SIZE)), PAGING_ACCESS_FROM_ALL | PAGING_IS_PRESENT | PAGING_IS_WRITEABLE);
}

int process_get_free_slot()
{
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (processes[i] == 0)
            return i;
    }
    //TODO: assert
    return -EISTKN;
}

int process_load_switch(const char* filename, struct process* process)
{
    int res = process_load(filename, process);
    if (res == 0)
    {
        process_switch(process);
    }
    return res;
}

int process_get_next_allocation_index(struct process* process) {
    for (int i = 0; i < PROCESS_MAX_ALLOCATIONS; i++) {
        if (process->allocations[i] == 0)
            return i;
    }
    return -ENOMEM;
}

int process_load_for_slot(const char* filename, struct process* process, int process_slot)
{
    int res = 0;
    struct task* task = 0;
    char* program_stack_ptr = nullptr;
    char* process_vbe_framebuffer = nullptr;
    auto vbe = static_cast<VBEGraphics*>(VBEGraphics::the());

    if (process_get(process_slot) != 0)
    {
        res = -EISTKN;
        goto out;
    }

    res = process_load_data(filename, process);
    if (ISERR(res))
        goto out;

    program_stack_ptr = (char*) kzalloc(PROGRAM_VIRTUAL_STACK_SIZE);
    if (!program_stack_ptr)
    {
        res = -ENOMEM;
        goto out;
    }

    strncpy(process->filename, filename, sizeof(process->filename));
    process->stack = program_stack_ptr;
    process->id = process_slot;

    task = task_new(process);
    if (ERROR_I(task) == 0)
    {
        res = ERROR_I(task);
        goto out;
    }

    process->task = task;

    res = process_map_memory(process);
    if (res < 0)
        goto out;

    processes[process_slot] = process;

    /* Framebuffer */
    process_vbe_framebuffer = static_cast<char*>(process_malloc(process, vbe->get_framebuffer_size()));
    process->framebuffer = process_vbe_framebuffer;
    /* */

    out:
        if (ISERR(res))
        {
            if (process->task)
                task_free(process->task);

            if (process->elf)
                delete static_cast<ELFFile *>(process->elf);

            if (process->framebuffer)
                process_free(process, process->framebuffer);

            if (process->bss)
                kfree(process->bss);
        }
        return res;
}

int process_load(const char* filename, struct process* process)
{
    int res = 0;

    int process_slot = process_get_free_slot();
    if (process_slot < 0)
    {
        //TODO: assert
        res = -EISTKN;
        goto out;
    }

    res = process_load_for_slot(filename, process, process_slot);
    out:
    return res;
}


void process_pushkey(struct process* process, char c)
{
    if (!process)
    {
        return;
    }

    int buffer_index = process->keyboard.tail % sizeof(process->keyboard.buffer);
    process->keyboard.buffer[buffer_index] = c;
    process->keyboard.tail++;
}

char process_popkey(struct process* process)
{
    if (!task_current())
    {
        return 0;
    }

    int buffer_index = process->keyboard.head % sizeof(process->keyboard.buffer);
    char c = process->keyboard.buffer[buffer_index];
    if (c == 0)
    {
        return 0;
    }
    process->keyboard.buffer[buffer_index] = 0;
    process->keyboard.head++;
    return c;
}

void* process_malloc(struct process* process, size_t size) {

    int next_alloc_idx = process_get_next_allocation_index(process);
    if (next_alloc_idx == -ENOMEM)
        return 0;

    void* ptr = kmalloc(size);

    process->allocations[next_alloc_idx] = ptr;

    int res = paging_map_to(process->task->page_directory, ptr, ptr,
        paging_align_address((void*)((int)ptr+size)),
        PAGING_IS_PRESENT | PAGING_IS_WRITEABLE | PAGING_ACCESS_FROM_ALL);

    if (res < 0) {
        kfree(ptr);
        return 0;
    }

    return process->allocations[next_alloc_idx];
}

void process_free(struct process* process, void* address) {
    for (int i = 0; i < PROCESS_MAX_ALLOCATIONS; i++) {
        if (process->allocations[i] == address) {
            kfree(process->allocations[i]);
            process->allocations[i] = 0;
            return;
        }
    }
}

void process_terminate(struct process* process) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i] == process) {
            processes[i] = 0;
        }
    }

    if (current_process == process) {
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (processes[i] != 0) {
                process_switch(processes[i]);
                break;
            }
        }
    }
    if (current_process == process)
        kernel_panic("No process to switch to!");


    for (int i = 0; i < PROCESS_MAX_ALLOCATIONS; i++) {
        if (process->allocations[i] == 0)
            continue;
        kfree(process->allocations[i]);
        process->allocations[i] = 0;
    }

    task_free(process->task);
    auto* elf = static_cast<ELFFile*>(process->elf);
    delete elf;

    kfree(process->stack);
    kfree(process->bss);
    delete process;
}

struct process** process_get_list()
{
    return processes;
}

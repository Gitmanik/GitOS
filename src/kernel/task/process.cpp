extern "C" {
#include "process.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "fs/file.h"
#include "common/status.h"
#include "task.h"
#include "common/string.h"
}
#include "formats/elf/ELFFile.hpp"

struct process* current_process = 0;

static struct process* processes[MAX_PROCESSES] = {};

static void process_init(struct process* process)
{
    memset(process, 0, sizeof(struct process));
}

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
    // TODO: SWAP VIDEO MEMORY
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
    int res = 0;

    auto* elf_file = static_cast<ELFFile *>(process->elf);

    res = paging_map_to(process->task->page_directory,  paging_align_address_to_lower_page(elf_file->get_virtual_base_address()), elf_file->get_physical_base_address(), paging_align_address(elf_file->get_physical_end_address()),
        PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE);

    res = paging_map_to(process->task->page_directory, (void*) (PROGRAM_VIRTUAL_STACK_ADDRESS_END), process->stack, paging_align_address((void*) ((uint32_t) process->stack + PROGRAM_VIRTUAL_STACK_SIZE)), PAGING_ACCESS_FROM_ALL | PAGING_IS_PRESENT | PAGING_IS_WRITEABLE);
    return res;
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

int process_load_switch(const char* filename, struct process** process)
{
    int res = process_load(filename, process);
    if (res == 0)
    {
        process_switch(*process);
    }
    return res;
}


int process_load_for_slot(const char* filename, struct process** process, int process_slot)
{
    int res = 0;
    struct task* task = 0;
    struct process* _process = nullptr;
    char* program_stack_ptr = nullptr;

    if (process_get(process_slot) != 0)
    {
        res = -EISTKN;
        goto out;
    }

    _process = new struct process;
    if (_process == nullptr)
    {
        res = -ENOMEM;
        goto out;
    }

    process_init(_process);
    res = process_load_data(filename, _process);
    if (ISERR(res))
        goto out;

    program_stack_ptr = (char*) kzalloc(PROGRAM_VIRTUAL_STACK_SIZE);
    if (!program_stack_ptr)
    {
        res = -ENOMEM;
        goto out;
    }

    strncpy(_process->filename, filename, sizeof(_process->filename));
    _process->stack = program_stack_ptr;
    _process->id = process_slot;

    task = task_new(_process);
    if (ERROR_I(task) == 0)
    {
        res = ERROR_I(task);
        goto out;
    }

    _process->task = task;

    res = process_map_memory(_process);
    if (res < 0)
        goto out;

    *process = _process;

    processes[process_slot] = _process;

    out:
        if (ISERR(res))
        {
            if (_process && _process->task)
            {
                task_free(_process->task);
            }
            // TODO: Free the process data
        }
        return res;
}

int process_load(const char* filename, struct process** process)
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
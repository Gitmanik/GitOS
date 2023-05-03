#include "process.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "fs/file.h"
#include "common/status.h"

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

int process_get(int process_id)
{
    if (process_id < 0 || process_id >= MAX_PROCESSES)
        return -EINVARG;

    return processes[process_id];
}

static int process_load_binary(const char* filename, struct process* process)
{
    int res = 0;

    int fd = fopen(filename, "r");
    if (!fd)
    {
        res = -EIO;
        goto out;
    }

    struct file_stat stat;
    res = fstat(fd, &stat);
    if (ISERR(res))
        goto out;

    void* program_data_ptr = kzalloc(stat.filesize);

    if (!program_data_ptr)
    {
        res = -ENOMEM;
        goto out;
    }

    if (fread(program_data_ptr, stat.filesize, 1, fd) != 1)
    {
        res = -EIO;
        goto out;
    }

    process->ptr = program_data_ptr;
    process->size= stat.filesize;

out:
    fclose(fd);
    return res;
}

static int process_load_data(const char* filename, struct process* process)
{
    int res = 0;
    res = process_load_binary(filename, process);
    return res;
}

int process_load_for_slot(const char* filename, struct process** process, int process_slot)
{
    int res = 0;
    struct task* task = 0;
    struct process* _process = 0;

    if (process_get(process_slot) != 0)
    {
        res = -EISTKN;
        goto out;
    }

    _process = kzalloc(sizeof(struct process));
    if (!_process)
    {
        res = -ENOMEM;
        goto out;
    }

    process_init(_process);
    res = process_load_data(filename, _process);
    if (ISERR(res))
        goto out;

    out:
        if (_process)
            kfree(_process);
        return res;
}
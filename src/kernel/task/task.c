#include "task.h"

#include <common/assert.h>

#include "common/status.h"
#include "memory/paging/paging.h"
#include "memory/heap/kheap.h"
#include "gdt/gdt.h"
#include "idt/idt.h"
#include "memory/memory.h"
#include "process.h"
#include "kernel.h"
#include "common/string.h"

struct task* current_task = 0;

struct task* task_tail = 0;
struct task* task_head = 0;

int task_init(struct task* task, struct process* process);

/**
 * @brief Returns currently running task
 * 
 * @return struct task* Current task
 */
struct task* task_current()
{
    return current_task;
}

void task_save_state(struct task* task, struct interrupt_frame* frame)
{
    task->registers.ip = frame->ip;
    task->registers.cs = frame->cs;
    task->registers.ss = frame->ss;
    task->registers.flags = frame->flags;
    task->registers.eax = frame->eax;
    task->registers.ebx = frame->ebx;
    task->registers.ecx = frame->ecx;
    task->registers.edx = frame->edx;
    task->registers.edi = frame->edi;
    task->registers.esi = frame->esi;
    task->registers.ebp = frame->ebp;
    task->registers.esp = frame->esp;

}

void task_current_save_state(struct interrupt_frame* frame)
{
    if (task_current() == 0)
    {
        kernel_panic("No current task to save!");
    }

    struct task* task = task_current();
    task_save_state(task, frame);
}

/**
 * @brief Returns next task in list (or first if there is no next task in current_task)
 * 
 * @return struct task* Next task
 */
struct task* task_get_next()
{
    if (!current_task->next)
        return task_head;
    return current_task->next;
}

static void task_list_remove(struct task* task)
{
    if (task->prev)
    {
        task->prev->next = task->next;
    }
    if (task == task_head)
    {
        task_head = task->next;
    }
    if (task == task_tail)
    {
        task_tail = task->prev;
    }

    if (task == current_task)
    {
        current_task = task_get_next();
    }
}

/**
 * @brief Frees all data associated with task struct 
 * 
 * @param task Task to free
 * @return int Error code
 */
int task_free(struct task* task)
{
    paging_free_directory(task->page_directory);
    task_list_remove(task);
    kfree(task);
    return 0;
}

/**
 * @brief Switch current task (switch pages)
 * 
 * @param task Task to switch to
 * @return int Error code
 */
int task_switch(struct task* task)
{
    current_task = task;
    paging_switch(task->page_directory);
    return 0;
}

/**
 * @brief Loads into the task's page
 * 
 * @return int Error code
 */
int task_page()
{
    user_registers();
    task_switch(current_task);
    return 0;
}

void task_run_first_ever_task()
{
    if (!current_task)
    {
        kernel_panic("No current task exists");
    }
    task_switch(task_head);
    task_return(&task_head->registers);
}

/**
 * @brief Initializes task struct's ip,ss,esp registers and created new paging directory
 * 
 * @param task Struct to be initialized
 * @return int Error code
 */
int task_init(struct task* task, struct process* process)
{
    memset(task, 0, sizeof(struct task));
    task->page_directory = paging_new_directory(PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

    if (!task->page_directory)
    {
        return -EIO;
    }
    task->registers.ip = (uint32_t) process->elf_entry;
    task->registers.ss = USER_DATA_SELECTOR;
    task->registers.cs = USER_CODE_SELECTOR;
    task->registers.esp = PROGRAM_VIRTUAL_STACK_ADDRESS_START;
    task->registers.flags = 0x200;

    task->process = process;

    return 0;
}


struct task* task_new(struct process* process)
{
    int res = 0;
    struct task* task = kzalloc(sizeof(struct task));

    if (!task)
    {
        res = -ENOMEM;
        goto out;
    }

    res = task_init(task, process);
    if (ISERR(res))
        goto out;
    
    if (task_head == 0)
    {
        task_head = task;
        task_tail = task;
        current_task = task;
        goto out;
    }

    task_tail->next = task;
    task->prev = task_tail;
    task_tail = task;

    out:
    if (ISERR(res))
    {
        task_free(task);
        task_tail = task;
        return ERROR(res);
    }
    return task;
}

int task_copy_string_from(struct task* task, void* virtual_address, void* physical_address, int max)
{
    // TODO: Allow more than one page size
    assert(PAGING_PAGE_SIZE > max);

    int res = 0;
    char* tmp = kzalloc(max);
    assert(tmp);

    uint32_t* task_directory = task->page_directory->directory_entry;
    uint32_t old_entry = paging_get_page(task_directory, tmp);
    paging_set_page(task->page_directory->directory_entry, tmp, (uint32_t) tmp | PAGING_IS_PRESENT | PAGING_IS_WRITEABLE | PAGING_ACCESS_FROM_ALL);
    paging_switch(task->page_directory);
    strncpy(tmp, virtual_address, max);
    kernel_page();

    res = paging_set_page(task_directory, tmp, old_entry);
    assert(res == 0);

    strncpy(physical_address, tmp, max);

    kfree(tmp);
    return res;
}

void task_page_task(struct task* task)
{
    user_registers();
    task_switch(task);
}

void* task_peek_stack(struct task* task, int offset)
{
    void* res = 0;

    uint32_t* sp_ptr = (uint32_t*) task->registers.esp;

    task_page_task(task);

    res = (void*) sp_ptr[offset];

    kernel_page();

    return res;
}

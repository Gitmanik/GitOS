#include "task.h"
#include "common/status.h"
#include "memory/paging/paging.h"
#include "memory/heap/kheap.h"
#include "gdt/gdt.h"
#include "memory/memory.h"
#include "process.h"
#include "kernel.h"

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
    paging_switch(task->page_directory->directory_entry);
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
    task->registers.ip = PROGRAM_VIRTUAL_ADDRESS;
    task->registers.ss = USER_DATA_SELECTOR;
    task->registers.cs = USER_CODE_SELECTOR;
    task->registers.esp = PROGRAM_VIRTUAL_STACK_ADDRESS_START;

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
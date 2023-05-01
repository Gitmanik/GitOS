#include "task.h"
#include "common/status.h"
#include "memory/paging/paging.h"
#include "memory/heap/kheap.h"
#include "gdt/gdt.h"
#include "memory/memory.h"

struct task* current_task = 0;

struct task* task_tail = 0;
struct task* task_head = 0;

int task_init(struct task* task);

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
 * @brief Initializes task struct's ip,ss,esp registers and created new paging directory
 * 
 * @param task Struct to be initialized
 * @return int Error code
 */
int task_init(struct task* task)
{
    memset(task, 0, sizeof(struct task));
    task->page_directory = paging_new_directory(PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

    if (!task->page_directory)
    {
        return -EIO;
    }
    task->registers.ip = PROGRAM_VIRTUAL_ADDRESS;
    task->registers.ss = USER_DATA_SELECTOR;
    task->registers.esp = PROGRAM_VIRTUAL_STACK_ADDRESS_START;

    return 0;
}


struct task* task_new()
{
    int res = 0;
    struct task* task = kzalloc(sizeof(struct task));

    if (!task)
    {
        res = -ENOMEM;
        goto out;
    }

    res = task_init(task);
    if (ISERR(res))
        goto out;
    
    if (task_head == 0)
    {
        task_head = task;
        task_tail = task;
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
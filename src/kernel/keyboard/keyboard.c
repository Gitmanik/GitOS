#include "keyboard.h"
#include "common/status.h"
#include "task/process.h"
#include "task/task.h"


static struct keyboard* keyboard_list_head = 0;
static struct keyboard* keyboard_list_tail = 0;

int keyboard_insert(struct keyboard* keyboard)
{
    int res = 0;
    if (keyboard->init == 0)
    {
        res = -EINVARG;
        goto out;
    }

    if (keyboard_list_tail)
    {
        keyboard_list_tail->next = keyboard;
        keyboard_list_tail = keyboard;
    }
    else
    {
        keyboard_list_head = keyboard;
        keyboard_list_tail = keyboard;
    }

    keyboard->init();

    out:
    return res;
}

void keyboard_push(char c)
{
    struct process* process = process_current();
    if (!process)
    {
        return;
    }

    int buffer_index = process->keyboard.tail % sizeof(process->keyboard.buffer);
    process->keyboard.buffer[buffer_index] = c;
    process->keyboard.tail++;
}

char keyboard_pop()
{
    if (!task_current())
    {
        return 0;
    }

    struct process* process = task_current()->process;
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
#pragma once

#include "memory/paging/paging.h"

struct registers
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t esp;
    uint32_t ss;
};

struct task 
{
    /**
     * @brief The page directory of the task
     */
    struct paging_chunk* page_directory;

    /**
     * @brief Stored registers of the task (when the task is not running)
     * 
     */
    struct registers registers;

    /**
     * @brief Next task 
     */
    struct task* next;

    /**
     * @brief Previous task
     */
    struct task* prev;
};

#define PROGRAM_VIRTUAL_ADDRESS 0x400000
#define PROGRAM_VIRTUAL_STACK_SIZE 1024 * 16
#define PROGRAM_VIRTUAL_STACK_ADDRESS_START 0x3FF000
#define PROGRAM_VIRTUAL_STACK_ADDRESS_END  PROGRAM_VIRTUAL_STACK_ADDRESS_START - PROGRAM_VIRTUAL_STACK_SIZE

struct task* task_current();
struct task* task_new();
struct task* task_get_next();
int task_free(struct task* task);
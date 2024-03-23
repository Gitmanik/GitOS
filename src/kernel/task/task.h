#pragma once

#include "memory/paging/paging.h"

struct registers
{
    uint32_t edi; //0
    uint32_t esi; //4
    uint32_t ebp; //8
    uint32_t ebx; //12
    uint32_t edx; //16
    uint32_t ecx; //20
    uint32_t eax; //24

    uint32_t ip; //28
    uint32_t cs; //32
    uint32_t flags; //36
    uint32_t esp; //40
    uint32_t ss; //44
} __attribute__((packed));

struct process;

struct task 
{
    /**
     * @brief The page directory of the task
     * 
     */
    struct paging_chunk* page_directory;

    /**
     * @brief Stored registers of the task (when the task is not running)
     * 
     */
    struct registers registers;

    /**
     * @brief Process of task
     * 
     */
    struct process* process;

    /**
     * @brief Next task 
     * 
     */
    struct task* next;

    /**
     * @brief Previous task
     * 
     */
    struct task* prev;
};

#define PROGRAM_VIRTUAL_ADDRESS 0x400000
#define PROGRAM_VIRTUAL_STACK_SIZE 1024 * 16
#define PROGRAM_VIRTUAL_STACK_ADDRESS_START 0x3FF000
#define PROGRAM_VIRTUAL_STACK_ADDRESS_END  PROGRAM_VIRTUAL_STACK_ADDRESS_START - PROGRAM_VIRTUAL_STACK_SIZE

void* restore_general_purpose_registers(struct registers* registers);

struct task* task_current();
struct task* task_get_next();
struct task* task_new(struct process* process);
int task_free(struct task* task);
void task_return(struct registers* registers);

void task_run_first_ever_task();
void user_registers();
int task_switch(struct task* task);
int task_page();

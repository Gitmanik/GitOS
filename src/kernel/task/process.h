#pragma once

#include <stdint.h>
#include "fs/pathparser.h"

#define PROGRAM_MAX_ALLOCATIONS 1024
#define PROCESS_KEYBOARD_BUFFER_SIZE 1024
#define MAX_PROCESSES 12

struct process
{

    /**
     * @brief Process ID
     */
    uint16_t id;

    /**
     * @brief Path to process
     */
    char filename[MAX_PATH];

    /**
     * @brief Main process task
     */
    struct task* task;

    /**
     * @brief malloc allocations of the process
     */
    void* allocations[PROGRAM_MAX_ALLOCATIONS];

    // TODO: Make this ELFFile* when rewritten to C++
    void* elf;

    /**
     * @brief Physical pointer to the process stack
     */
    void* stack;

    struct keyboard_buffer {
        char buffer[PROCESS_KEYBOARD_BUFFER_SIZE];
        int tail;
        int head;
    } keyboard;

    void* elf_entry;
};
int process_load_switch(const char* filename, struct process** process);
int process_load(const char* filename, struct process** process);
struct process* process_current();
int process_switch(struct process* process);
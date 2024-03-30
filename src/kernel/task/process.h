#pragma once

#include <stdint.h>
#include "fs/pathparser.h"

#define PROGRAM_MAX_ALLOCATIONS 1024
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

    /**
     * @brief Physical pointer to the process memory
     */
    void* ptr;

    /**
     * @brief Physical pointer to the process stack
     */
    void* stack;

    /**
     * @brief Size of the ptr data
     */
    uint32_t size;
};

int process_load(const char* filename, struct process** process);
struct process* process_current();
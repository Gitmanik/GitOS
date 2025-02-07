#pragma once

#include <stdint.h>
#include "fs/pathparser.h"
#include <stddef.h>

#define PROCESS_MAX_ALLOCATIONS 1024
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
    void* allocations[PROCESS_MAX_ALLOCATIONS];

    // TODO: Make this ELFFile* when rewritten to C++
    void* elf;

    /**
     * @brief Physical pointer to the process stack
     */
    void* stack;

    /**
     * @brief Allocated BSS segment
     */
    void* bss;

    /**
     * @brief Pointer to allocated memory for framebuffer
     */
    void* framebuffer;

    struct keyboard_buffer {
        char buffer[PROCESS_KEYBOARD_BUFFER_SIZE];
        int tail;
        int head;
    } keyboard;

    void* elf_entry;

    int argc;
    char** argv;
};
int process_load_switch(const char* filename, struct process* process);
int process_load(const char* filename, struct process* process);
struct process* process_current();
int process_switch(struct process* process);
void process_pushkey(struct process* process, char c);
char process_popkey(struct process* process);
void* process_malloc_flags(struct process* process, size_t size, int flags);
void* process_malloc(struct process* process, size_t size);
void process_free(struct process* process, void* address);
void process_terminate(struct process* process);

struct process** process_get_list();

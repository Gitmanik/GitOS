#pragma once
#include <stdint.h>
#include <stddef.h>

#define HEAP_BLOCK_TABLE_ENTRY_TAKEN 0x01
#define HEAP_BLOCK_TABLE_ENTRY_FREE  0x00

#define HEAP_BLOCK_HAS_NEXT 0b10000000
#define HEAP_BLOCK_IS_FIRST  0b01000000

typedef unsigned char HEAP_BLOCK_TABLE_ENTRY;

typedef struct
{
    HEAP_BLOCK_TABLE_ENTRY* entries;
    size_t total;
} heap_table;

typedef struct
{
    heap_table* table;
    void* start_address;
} heap;

#define HEAP_TABLE_ADDRESS 0x00007e00
#define HEAP_BLOCK_SIZE 4096

int heap_create(heap* heap, void* ptr, void* end, heap_table* table);
void* heap_malloc(heap* heap, size_t size);
void heap_free(heap* heap, void* ptr);
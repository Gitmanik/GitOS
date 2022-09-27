#include <stdint.h>
#include "kheap.h"
#include "heap.h"

heap kernel_heap;
heap_table kernel_heap_table;

/**
 * @brief Initializes kernel heap
 * 
 * @param start_address Starting address for heap allocation
 * @param size Size of heap in bytes
 * @return int Status \file "common/status.h"
 */
int kheap_init(void* start_address, uint32_t size)
{
    int total_table_entries = size / HEAP_BLOCK_SIZE;
    kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY*) HEAP_TABLE_ADDRESS;
    kernel_heap_table.total = total_table_entries;

    void* end = start_address + size;
    return  heap_create(&kernel_heap, &kernel_heap_table, start_address, end);
}

/**
 * @brief Allocates specified size in heap
 * 
 * @param size Requested allocation size
 * @return void* Pointer to allocated memory, 0 if errored
 */
void* kmalloc(size_t size)
{
    return heap_malloc(&kernel_heap, size);
}

/**
 * @brief Frees specified pointer
 * 
 * @param ptr Pointer to free
 */
void kfree(void* ptr)
{
    heap_free(&kernel_heap, ptr);
}
#include <stdint.h>
#include "kheap.h"
#include "heap.h"
#include "../../kernel.h"

heap kernel_heap;
heap_table kernel_heap_table;

void kheap_init(void* start_address, uint32_t size)
{
    int total_table_entries = size / HEAP_BLOCK_SIZE;
    kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY*) HEAP_TABLE_ADDRESS;
    kernel_heap_table.total = total_table_entries;

    void* end = start_address + size;
    int res = heap_create(&kernel_heap, start_address, end, &kernel_heap_table);
    if (res < 0)
    {
        kernel_message("Panic: Failed to create heap!", RED);
        kernel_halt();
    }
}

void* kmalloc(size_t size)
{
    return heap_malloc(&kernel_heap, size);
}

void* kfree(void* ptr)
{
    heap_free(&kernel_heap, ptr);
}
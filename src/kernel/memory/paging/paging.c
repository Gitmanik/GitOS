#include "paging.h"
#include <stdint.h>
#include <stdbool.h>
#include "memory/heap/kheap.h"
#include "common/status.h"

static uint32_t* current_directory = 0;

/**
 * @brief Ensures provided address is aligned
 * 
 * @param addr Address
 * @return bool Address is aligned
 */
static bool paging_is_aligned(void* addr)
{
    return ((uint32_t) addr % PAGING_PAGE_SIZE) == 0;
}

/**
 * @brief Calculates directory and page table index for provided virtual address
 * 
 * @param virtual_address Virtual address
 * @param directory_index_out Directory index (output)
 * @param page_table_index_out Page table index (output)
 * @return int Status
 */
static int paging_get_indexes(void* virtual_address, uint32_t* directory_index_out, uint32_t* page_table_index_out)
{
    if (!paging_is_aligned(virtual_address))
        return -EINVARG;

    *directory_index_out = ((uint32_t) virtual_address / (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE));
    *page_table_index_out = ((uint32_t) virtual_address % (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE) / PAGING_PAGE_SIZE);

    return 0;
}

/**
 * @brief Loads given page directory pointer to cr3 CPU register
 * 
 * @param directory Page directory pointer
 */
static void paging_load_directory(uint32_t* directory)
{
    asm volatile ("mov %0, %%cr3" : : "r" (directory));
}

/**
 * @brief Returns directory entry pointer for given chunk
 * 
 * @param chunk Chunk
 * @return uint32_t* Page directory pointer
 */
uint32_t* paging_get_directory(struct paging_chunk* chunk)
{
    return chunk->directory_entry;
}

/**
 * @brief Allocates and creates new page directory with specified flags
 * 
 * @param flags Flags for each page table
 * @return paging_chunk* Pointer to new paging chunk
 */
struct paging_chunk* paging_new_directory(uint8_t flags)
{
    uint32_t* page_directories = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
    int offset = 0;
    for (int i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++)
    {
        uint32_t* page_table = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
        for (int j = 0; j < PAGING_TOTAL_ENTRIES_PER_TABLE; j++)
        {
            page_table[j] = (offset + (j * PAGING_PAGE_SIZE)) | flags;
        }
        page_directories[i] = (uint32_t) page_table | flags | PAGING_IS_WRITEABLE;
        offset += (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE);
    }

    struct paging_chunk* chunk = kzalloc(sizeof(struct paging_chunk));
    chunk->directory_entry = page_directories;
    return chunk;
}

/**
 * @brief Switches to given page directory pointer
 * 
 * @param directory Page directory pointer
 */
void paging_switch(uint32_t* directory)
{
    paging_load_directory(directory);
    current_directory = directory;
}

/**
 * @brief Sets page table entry
 * 
 * @param directory Page directory pointer
 * @param virtual_address Virtual address
 * @param value Value for page table entry
 * @return int Status
 */
int paging_set_page(uint32_t* directory, void* virtual_address, uint32_t value)
{
    if (!paging_is_aligned(virtual_address))
        return -EINVARG;

    uint32_t directory_index, page_table_index;

    int res = paging_get_indexes(virtual_address, &directory_index, &page_table_index);

    if (res < 0)
        return res;

    uint32_t page_directory_entry = directory[directory_index];
    uint32_t* page_table = (uint32_t*) (page_directory_entry & 0xfffff000);
    page_table[page_table_index] = value;

    return 0;
}

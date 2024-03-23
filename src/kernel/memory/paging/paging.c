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
 * @brief Frees all entries and page directory itself
 * 
 * @param chunk Page directory to free
 */
void paging_free_directory(struct paging_chunk* chunk)
{
    for (int i = 0; i <1024; i++)
    {
        uint32_t entry = chunk->directory_entry[i];
        uint32_t* table = ((uint32_t*)(entry & 0xfffff000)); //lowest 12 bits are flags
        kfree(table);
    }
    kfree(chunk->directory_entry);
    kfree(chunk);
}

/**
 * @brief Switches to given page directory pointer
 * 
 * @param chunk Page directory pointer
 */
void paging_switch(struct paging_chunk* chunk)
{
    paging_load_directory(chunk->directory_entry);
    current_directory = chunk->directory_entry;
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

/**
 * @brief Aligns pointer to page size
 * 
 * @param ptr 
 * @return void* 
 */
void* paging_align_address(void* ptr)
{
    if ((uint32_t) ptr % PAGING_PAGE_SIZE)
    {
        return (void*)((uint32_t) ptr + PAGING_PAGE_SIZE - ((uint32_t) ptr % PAGING_PAGE_SIZE));
    }
    return ptr;
}

/**
 * @brief Maps virtual address to physical in page directory
 * 
 * @param chunk Paging chunk
 * @param physical Physical address
 * @param flags Page directory entry flags
 * @return int Error code
 */
int paging_map(struct paging_chunk* chunk, void* virtual, void* physical, int flags)
{
    if (((unsigned int) virtual % PAGING_PAGE_SIZE) || ((unsigned int) physical % PAGING_PAGE_SIZE))
    {
        return -EINVARG;
    }

    return paging_set_page(chunk->directory_entry, virtual, (uint32_t) physical | flags);
}

/**
 * @brief Maps range of virtual addresses to physical in page directory
 * 
 * @param directory Page directory
 * @param physical Physical address
 * @param flags Page directory entry flags
 * @param count Count of entries
 * @return int Error code
 */
int paging_map_range(struct paging_chunk* directory, void* virtual, void* physical, int count, int flags)
{
    int res = 0;
    for (int i = 0; i < count; i++)
    {
        res = paging_map(directory, virtual, physical, flags);
        if (res == 0)
            break;
        virtual += PAGING_PAGE_SIZE;
        physical += PAGING_PAGE_SIZE;
    }
    return res;
}

/**
 * @brief Maps virtual address to physical in page directory
 * 
 * @param chunk Paging chunk
 * @param physical Physical address start
 * @param physical_end Physical address end
 * @param flags Page directory entry flags
 * @return int Error code
 */
int paging_map_to(struct paging_chunk* chunk, void* virtual, void* physical, void* physical_end, int flags)
{
    int res = 0;
    if ((uint32_t) virtual % PAGING_PAGE_SIZE)
    {
        res = -EINVARG;
        goto out;
    }
    if ((uint32_t) physical % PAGING_PAGE_SIZE)
    {
        res = -EINVARG;
        goto out;
    }
    if ((uint32_t) physical_end % PAGING_PAGE_SIZE)
    {
        res = -EINVARG;
        goto out;
    }

    if ((uint32_t) physical_end < (uint32_t) physical)
    {
        res = -EINVARG;
        goto out;
    }

    uint32_t total_bytes = physical_end-physical;
    int total_pages = total_bytes / PAGING_PAGE_SIZE;
    res = paging_map_range(chunk, virtual, physical, total_pages, flags);

    out:
    return res;
}
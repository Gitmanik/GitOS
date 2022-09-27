#include "heap.h"
#include "../../common/status.h"
#include "../memory.h"

/**
 * @brief Validates heap_table structure
 * 
 * @param ptr Starting address
 * @param end Ending address
 * @param table Heap table
 * @return int Status
 */
static int heap_validate_table(void* ptr, void* end, heap_table* table)
{
    size_t table_size = (size_t) end - (size_t) ptr;
    size_t total_blocks = table_size / HEAP_BLOCK_SIZE;
    if (table->total != total_blocks)
        return -EINVARG;
    return 0;
}

/**
 * @brief Makes sure that pointer is aligned
 * 
 * @param ptr Pointer
 * @return int 0 if aligned
 */
static int heap_validate_alignment(void* ptr)
{
    return ((uint32_t)ptr % HEAP_BLOCK_SIZE) == 0;
}

/**
 * @brief Upper aligns size
 * 
 * @param val Size to align
 * @return uint32_t Aligned size
 */
static uint32_t heap_align_size(uint32_t val)
{
    if ((val % HEAP_BLOCK_SIZE) == 0) return val;

    val = (val - (val % HEAP_BLOCK_SIZE)) + HEAP_BLOCK_SIZE;
    return val;
}

/**
 * @brief Extracts entry type
 * 
 * @param entry Entry to extract
 * @return int Type
 */
static int heap_get_entry_type(HEAP_BLOCK_TABLE_ENTRY entry)
{
    return entry & 0x0f;
}

/**
 * @brief Converts block number to address
 * 
 * @param heap Heap to manage
 * @param block Block to convert
 * @return void* Address
 */
static void* heap_block_to_address(heap* heap, int block)
{
    return heap->start_address + HEAP_BLOCK_SIZE * block;
}

/**
 * @brief Converts pointer to block number
 * 
 * @param heap Heap to manage
 * @param ptr Address to convert
 * @return int Block number
 */
static int heap_address_to_block(heap* heap, void* ptr)
{
    return (ptr - heap->start_address) / HEAP_BLOCK_SIZE;
}

/**
 * @brief Finds first free set of blocks in heap
 * 
 * @param heap Heap to manage
 * @param total_blocks Amount of requestes blocks
 * @return int Status
 */
static int heap_get_start_block(heap* heap, int total_blocks)
{
    int current_block = 0;
    int start_block = -1;

    for (size_t i = 0; i < heap->table->total; i++)
    {
        HEAP_BLOCK_TABLE_ENTRY entry = heap->table->entries[i];
        if (heap_get_entry_type(entry) != HEAP_BLOCK_TABLE_ENTRY_FREE)
        {
            current_block = 0;
            start_block = -1;
            continue;
        }

        if (start_block == -1) //First block
        {
            start_block = i;
        }
            current_block++;
        if (current_block == total_blocks)
            break;
    }

    if (start_block == -1)
        return -ENOMEM;

    return start_block;
}

/**
 * @brief Marks blocks taken
 * 
 * @param heap Heap to manage
 * @param start_block Starting block to mark taken
 * @param total_blocks Total amount of blocks to mark taken
 */
void heap_mark_blocks_taken(heap* heap, int start_block, int total_blocks)
{
    for (int i = 0; i < total_blocks; i++)
    {
        heap->table->entries[start_block + i] |= HEAP_BLOCK_TABLE_ENTRY_TAKEN;
        if (i + 1 < total_blocks)
            heap->table->entries[start_block + i] |= HEAP_BLOCK_HAS_NEXT;
    }
}

/**
 * @brief Marks blocks free
 * 
 * @param heap Heap to manage
 * @param start_block Starting block to mark free
 */
static void heap_mark_blocks_free(heap* heap, int start_block)
{
    for (int i = start_block; i < heap->table->total; i++)
    {
        HEAP_BLOCK_TABLE_ENTRY entry = heap->table->entries[i];

        heap->table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;

        if (!(entry & HEAP_BLOCK_HAS_NEXT))
            break;
    }
}

/**
 * @brief Allocates blocks in heap
 * 
 * @param heap Heap to manage
 * @param total_blocks Requested amount of blocks
 * @return void* Pointer to allocated memory
 */
static void* heap_malloc_blocks(heap* heap, int total_blocks)
{
    void* ptr = 0;

    int start_block = heap_get_start_block(heap, total_blocks);
    if (start_block < 0)
        return 0;

    ptr = heap_block_to_address(heap, start_block);

    heap_mark_blocks_taken(heap, start_block, total_blocks);

    return ptr;
}

/**
 * @brief Creates heap in specified chunk of memory
 * 
 * @param heap Heap to manage
 * @param table Heap table to manage
 * @param ptr Starting address
 * @param end Ending address
 * @return int Status
 */
int heap_create(heap* heap, heap_table* table, void* ptr, void* end)
{
    if (!heap_validate_alignment(ptr) || !heap_validate_alignment(end))
        return -EINVARG;

    heap->start_address = ptr;
    heap->table = table;

    if (heap_validate_table(ptr, end, table) != 0)
        return -EINVARG;

    size_t table_size = sizeof(HEAP_BLOCK_TABLE_ENTRY) * table->total;
    memset(table->entries, HEAP_BLOCK_TABLE_ENTRY_FREE, table_size);

    return 0;
}

/**
 * @brief Allocates memory in heap
 * 
 * @param heap Heap to manage
 * @param size Requested allocation size
 * @return void* Pointer to allocated memory, 0 if errored
 */
void* heap_malloc(heap* heap, uint32_t size)
{
    uint32_t aligned_size = heap_align_size(size);
    int total_blocks = aligned_size / HEAP_BLOCK_SIZE;
    
    return heap_malloc_blocks(heap, total_blocks);
}
/**
 * @brief Frees specified pointer in heap
 * 
 * @param heap Heap to manage
 * @param ptr Pointer to free
 */
void heap_free(heap* heap, void* ptr)
{
    heap_mark_blocks_free(heap, heap_address_to_block(heap, ptr));
    return 0;
}
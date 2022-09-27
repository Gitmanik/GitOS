#include "memory.h"
#include <stddef.h>

/**
 * @brief Sets first bytes of memory pointed to specified value
 * 
 * @param ptr Pointer to the block of memory to fill
 * @param c Value to be set
 * @param size Number of bytes to set
 * @return void* Pointer to the block of memory
 */
void* memset(void* ptr, int c, size_t size)
{
    char* c_ptr = (char*) ptr;
    for (size_t i = 0; i < size; i++)
    {
        c_ptr[i] = (char) c;
    }
    return ptr;
}
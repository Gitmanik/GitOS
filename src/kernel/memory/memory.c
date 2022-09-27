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

/**
 * @brief Copies memory from one place to another
 * 
 * @param dstptr Destination pointer
 * @param srcptr Source pointer
 * @param size Length of copied memory
 * @return void* Destination pointer
 */
void* memcpy(void* restrict dstptr, const void* restrict srcptr, size_t size)
{
 	unsigned char* dst = (unsigned char*) dstptr;
 	const unsigned char* src = (const unsigned char*) srcptr;
	for (size_t i = 0; i < size; i++)
		dst[i] = src[i];
	return dstptr;
}

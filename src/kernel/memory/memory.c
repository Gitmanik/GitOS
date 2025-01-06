#include "memory.h"
#include <stddef.h>
#include <stdbool.h>

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
void* memcpy(void* dstptr, const void* srcptr, size_t size)
{
 	unsigned char* dst = (unsigned char*) dstptr;
 	const unsigned char* src = (const unsigned char*) srcptr;
	for (size_t i = 0; i < size; i++)
		dst[i] = src[i];
	return dstptr;
}

/**
 * @brief Compares the first num bytes of the block of memory pointed by ptr1 to the first num bytes pointed by ptr2.
 * 
 * @param ptr1 Pointer to block of memory
 * @param ptr2 Pointer to block of memory
 * @param len Number of bytes to compare
 * @return int
 * - 0 if matches,
 * - -1 the first byte that does not match in both memory blocks has a lower value in ptr1 than in ptr2,
 * - 1 the first byte that does not match in both memory blocks has a greater value in ptr1 than in ptr2
 */
int memcmp(void* ptr1, void* ptr2, size_t len)
{
    unsigned char* c1 = ptr1;
    unsigned char* c2 = ptr2;
    
    while (len-- > 0)
    {
        if (*c1++ != *c2++)
            return c1[-1] < c2[-1] ? -1 : 1;
    }
    return 0;
}
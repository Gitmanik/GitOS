#pragma once
#include <stdint.h>

typedef struct __attribute__ ((packed))
{
    uint64_t base_address;
    uint64_t length_in_bytes;
    uint32_t type;
} memory_map_entry;

/**
 * @brief Stage1 loads memory map to this address
 * 
 */
memory_map_entry* bios_memory_map = (memory_map_entry*) 0x500;
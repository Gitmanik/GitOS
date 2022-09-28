#pragma once
#include <stdint.h>

#define PAGING_CACHE_DISABLED  0b00010000
#define PAGING_WRITE_THROUGH   0b00001000
#define PAGING_ACCESS_FROM_ALL 0b00000100
#define PAGING_IS_WRITEABLE    0b00000010
#define PAGING_IS_PRESENT      0b00000001

#define PAGING_TOTAL_ENTRIES_PER_TABLE 1024
#define PAGING_PAGE_SIZE 4096

typedef struct
{
    uint32_t* directory_entry;

} paging_chunk;

paging_chunk* paging_new_directory(uint8_t flags);
void paging_switch(uint32_t* directory);
void paging_enable();
uint32_t* paging_get_directory(paging_chunk* chunk);
int paging_set_page(uint32_t* directory, void* virtual_address, uint32_t value);
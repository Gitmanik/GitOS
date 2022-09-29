#pragma once

/**
 * @brief Represents a real physical hard disk
 * 
 */
#define DISK_TYPE_REAL 0

#define DISK_SECTOR_SIZE 512

struct disk
{
    unsigned int disk_type;
    unsigned long sector_size;
};

void disk_search_and_init();
int disk_read_block(struct disk* disk, unsigned int lba, int total, void* buf);
struct disk* disk_get(int index);

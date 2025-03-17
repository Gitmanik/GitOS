#pragma once

/**
 * @brief Represents a real physical hard disk
 * 
 */
#define DISK_TYPE_REAL 0

/**
 * @brief Represents disk sector size
 * 
 */
#define DISK_SECTOR_SIZE 512
#include <stdint.h>

typedef unsigned int DISK_TYPE;

struct disk
{
    DISK_TYPE disk_type;
    int sector_size;
    int id;

    struct filesystem* filesystem;
    void* fs_private; 
};

struct partition_entry {
    uint8_t status;
    uint8_t chs_first[3];
    uint8_t type;
    uint8_t chs_last[3];
    uint32_t starting_lba;
    uint32_t size_in_sectors;
} __attribute__((packed));

void disk_search_and_init();
int disk_read_block(struct disk* disk, unsigned int lba, int total, void* buf);
struct disk* disk_get(int index);

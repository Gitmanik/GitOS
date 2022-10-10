#pragma once

#include <stdint.h>
#include "fs/file.h"
#include "drivers/disk/disk.h"
#include "drivers/disk/disk_streamer.h"

#define FAT16_SIGNATURE 0x29
#define FAT16_FAT_ENTRY_SIZE 0x20
#define FAT16_BAD_SECTOR 0xFF7
#define FAT16_UNUSED 0x00

typedef unsigned int FAT_ITEM_TYPE;
#define FAT_ITEM_TYPE_DIRECTORY 0
#define FAT_ITEM_TYPE_FILE      1

// FAT Directory Entry attributes bitmask
#define FAT_FILE_READONLY     0x01
#define FAT_FILE_HIDDEN       0x02
#define FAT_FILE_SYSTEM       0x04
#define FAT_FILE_VOLUME_LABEL 0x08
#define FAT_FILE_SUBDIRECTORY 0x10
#define FAT_FILE_ARCHVED      0x20
#define FAT_FILE_DEVICE       0x40
#define FAT_FILE_RESERVED     0x80

struct fat16_header_primary
{
    uint8_t jmp_short[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entries;
    uint16_t numeber_of_sectors;
    uint8_t media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t sectors_big;
} __attribute__((packed));

struct fat16_header_extended
{
    uint8_t drive_number;
    uint8_t win_nt_bit;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_id_string[11];
    uint8_t system_id_string[8];
} __attribute__((packed));

struct fat_header
{
    struct fat16_header_primary primary;
    union fat_header_extended
    {
        struct fat16_header_extended extended;
    } shared;
};

struct fat_directory_item
{
    uint8_t filename[8];
    uint8_t ext[3];
    uint8_t attribute;
    uint8_t reserved;
    uint8_t creation_time_tenths_of_a_sec;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access;
    uint16_t high_16bits_first_cluster;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t low_16bits_first_cluster;
    uint32_t filesize;
} __attribute__((packed));

struct fat_directory
{
    struct fat_directory_item* item;
    int total;
    int sector_pos;
    int ending_sector_pos;
};

struct fat_item
{
    union 
    {
        struct fat_directory_item* item;
        struct fat_directory* directory;
    };
    
    FAT_ITEM_TYPE type;
};

struct fat_file_descriptor
{
    struct fat_item* item;
    uint32_t pos;
};

struct fat_private
{
    struct fat_header header;
    struct fat_directory root_directory;

    //Used to stream data clusters
    struct disk_stream* cluster_read_stream;
    //Used to stream the file allocation table
    struct disk_stream* fat_read_stream;

    // Used to situations where we stream the directory
    struct disk_stream* directory_stream;
};

struct filesystem* fat16_init();
int fat16_resolve(struct disk* disk);
void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode);
int fat16_read(struct disk* disk, void* descriptor_buffer, uint32_t size, uint32_t nmemb, char* out);
#include "disk.h"
#include "common/io.h"
#include "memory/memory.h"
#include "common/status.h"
#include "fs/file.h"

struct disk primary_disk;

/**
 * @brief Reads sectors into memory
 * 
 * @param lba Starting LBA
 * @param total Count of sectors to read
 * @param buf Destination buffer
 * @return int Status
 */
static int disk_read_sector(int lba, int total, void* buf)
{
    outb(0x1F6, (lba >> 24) | 0xE0);
    outb(0x1F2 , total);
    outb(0x1F3, (unsigned char) (lba & 0xff));
    outb(0x1F4, (unsigned char) (lba >> 8));
    outb(0x1F5, (unsigned char) (lba >> 16));
    outb(0x1F7, 0x20);

    unsigned short* ptr = (unsigned short*) buf;
    for (int i = 0; i < total; i++)
    {
        //Wait for the buffer to be ready
        char c = inb(0x1F7);
        while (!(c & 0x08))
        {
            c = inb(0x1F7);
        }

        //Copy from hard disk to memory
        for (int j = 0; j < 256; j++)
        {
            *ptr = inw(0x1F0);
            ptr++;
        }
    }
    return 0;
}

void disk_search_and_init()
{
    memset(&primary_disk, 0, sizeof(struct disk));
    primary_disk.disk_type = DISK_TYPE_REAL;
    primary_disk.sector_size = DISK_SECTOR_SIZE;
    primary_disk.filesystem = fs_resolve(&primary_disk);
    primary_disk.id = 0;
}

struct disk* disk_get(int index)
{
    if (index != 0) //TODO Make an array
        index = 0;

    return &primary_disk;
}

int disk_read_block(struct disk* disk, unsigned int lba, int total, void* buf)
{
    if (disk != &primary_disk)
    {
        return -EIO;
    }

    return disk_read_sector(lba, total, buf);
}
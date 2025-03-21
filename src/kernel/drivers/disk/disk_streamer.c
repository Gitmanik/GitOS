#include "disk_streamer.h"

#include <kernel.h>

#include "memory/heap/kheap.h"
#include "disk.h"
#include <stdbool.h>
#include <common/assert.h>

/**
 * @brief Allocates and sets up new disk_stream struct
 * 
 * @param disk_id Disk ID
 * @return struct disk_stream* Created struct
 */
struct disk_stream* diskstreamer_new(int disk_id)
{
    struct disk* disk = disk_get(disk_id);
    if (!disk)
        return 0;

    struct disk_stream* streamer = kzalloc(sizeof(struct disk_stream));
    streamer->disk = disk;
    streamer->pos = 0;

    return streamer;
}

/**
 * @brief Seeks into specified offset in stream
 * 
 * @param stream Stream to seek
 * @param pos Bytes from 0
 * @return int 0
 */
int diskstreamer_seek(struct disk_stream* stream, int pos)
{
    stream->pos = pos;
    return 0;
}

/**
 * @brief Reads specified amount of bytes into buffer
 * 
 * @param stream Stream to read
 * @param out Target buffer
 * @param total Total amount of bytes to read
 * @return int Status
 */
int diskstreamer_read(struct disk_stream* stream, void* out, int total)
{
    bool buf_overflow;
    char buf[DISK_SECTOR_SIZE];
    do {
        uint32_t sector = stream->pos / DISK_SECTOR_SIZE;
        uint32_t offset = stream->pos % DISK_SECTOR_SIZE;
        uint32_t total_to_read = total;
        buf_overflow = offset+total >= DISK_SECTOR_SIZE;
        if (buf_overflow)
            total_to_read -= (offset+total_to_read) - DISK_SECTOR_SIZE;

        int res = disk_read_block(stream->disk, sector, 1, buf);
        assert(res == 0);

        for (uint32_t i = 0; i < total_to_read; i++)
        {
            *((char*) out) = buf[offset+i];
            out++;
        }

        stream->pos += total_to_read;
        total = total - total_to_read;
    } while (buf_overflow);

    return 0;
}

/**
 * @brief Frees allocated disk_stream
 * 
 * @param stream Stream to close
 */
void diskstreamer_close(struct disk_stream* stream)
{
    kfree(stream);
}

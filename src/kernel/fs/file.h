#pragma once

#include <stdint.h>
#include "drivers/disk/disk.h"
#include "pathparser.h"

#define MAX_FILESYSTEMS 12
#define MAX_FILEDESCRIPTORS 1024
#define MAX_FILESYSTEM_NAME 20

typedef unsigned int FILE_SEEK_MODE;
typedef unsigned int FILE_MODE;

enum {
    SEEK_SET,
    SEEK_CUR,
    SEEK_END
};

enum {
    FILE_MODE_READ,
    FILE_MODE_WRITE,
    FILE_MODE_APPEND,
    FILE_MODE_INVALID
};


typedef void* (*FS_OPEN_FUNCTION)(struct disk* disk, struct path_part* path, FILE_MODE mode);
typedef int (*FS_READ_FUNCTION)(struct disk* disk, void* descriptor, uint32_t size, uint32_t nmemb, char* out);
typedef int (*FS_RESOLVE_FUNCTION)(struct disk* disk);

struct filesystem
{
    FS_RESOLVE_FUNCTION resolve;
    FS_OPEN_FUNCTION open;
    FS_READ_FUNCTION read;

    char name[MAX_FILESYSTEM_NAME];
};

struct file_descriptor
{
    int index;
    struct filesystem* filesystem;
    void* private_buffer;
    struct disk* disk;
};

void fs_init();
int fopen(const char* filename, const char* mode);
int fread(void* ptr, uint32_t size, uint32_t nmemb, int fd);
void fs_insert_filesystem(struct filesystem* filesystem);
struct filesystem* fs_resolve(struct disk* disk);

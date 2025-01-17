#pragma once

#include <stdint.h>
#include "drivers/disk/disk.h"
#include "pathparser.h"

#define MAX_FILESYSTEMS 12
#define MAX_FILEDESCRIPTORS 1024
#define MAX_FILESYSTEM_NAME 20

typedef unsigned int FILE_SEEK_MODE;
typedef unsigned int FILE_MODE;
typedef unsigned int FILE_STAT_FLAGS;

enum FILE_SEEK_MODES
{
    /**
     * @brief Absolute position from 0
     * 
     */
    SEEK_SET,

    /**
     * @brief Relative position
     * 
     */
    SEEK_CUR,

    /**
     * @brief Absolute position from the end of file
     * 
     */
    SEEK_END
};

enum FILE_OPEN_MODES
{
    FILE_MODE_READ,
    FILE_MODE_WRITE,
    FILE_MODE_APPEND,
    FILE_MODE_INVALID
};

enum FILE_STAT_FLAGS_ENUM
{
    FILE_STAT_READ_ONLY = 0b00000001,
    FILE_STAT_FOLDER = 0b00000010
};

struct file_descriptor
{
    int index;
    struct filesystem* filesystem;
    void* private_fs_descriptor;
    struct disk* disk;
};

struct file_stat
{
    FILE_STAT_FLAGS flags;
    uint32_t filesize;
};

typedef int (*FS_RESOLVE_FUNCTION)(struct disk* disk);
typedef void* (*FS_OPEN_FUNCTION)(void* private_fs, struct path_part* path, FILE_MODE mode);
typedef int (*FS_READ_FUNCTION)(void* private_fs, void* descriptor, uint32_t size, uint32_t nmemb, char* out);
typedef int (*FS_SEEK_FUNCTION)(void* private_fs, uint32_t offset, FILE_SEEK_MODE seek_mode);
typedef int (*FS_STAT_FUNCTION)(void* private_fs, struct file_stat* stat);
typedef int (*FS_CLOSE_FUNCTION)(void* private_fs);


struct filesystem
{
    FS_RESOLVE_FUNCTION resolve;
    FS_OPEN_FUNCTION open;
    FS_READ_FUNCTION read;
    FS_SEEK_FUNCTION seek;
    FS_STAT_FUNCTION stat;
    FS_CLOSE_FUNCTION close;

    char name[MAX_FILESYSTEM_NAME];
};

void fs_init();
void fs_insert_filesystem(struct filesystem* filesystem);
struct filesystem* fs_resolve(struct disk* disk);

int fopen(const char* filename, const char* mode);
int fread(void* ptr, uint32_t size, uint32_t nmemb, int fd);
int fseek(int fd, int offset, FILE_SEEK_MODE whence);
int fstat(int fd, struct file_stat* stat);
int fclose(int fd);
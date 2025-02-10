//
// Created by Paweł Reich on 2/5/25.
//

#pragma once

#include <stdint-gcc.h>

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

struct file_stat
{
    FILE_STAT_FLAGS flags;
    uint32_t filesize;
};

int fopen(const char* filename, const char* mode);
int fread(void* ptr, uint32_t size, uint32_t nmemb, int fd);
int fwrite(void* ptr, uint32_t size, uint32_t nmemb, int fd);
int fstat(int fd, struct file_stat* stat);
int fseek(int fd, int offset, FILE_SEEK_MODE whence);
int fclose(int fd);

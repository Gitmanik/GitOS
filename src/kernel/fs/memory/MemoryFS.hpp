//
// Created by Paweł Reich on 2/7/25.
//

#pragma once

#include <stdint-gcc.h>
#include <stddef.h>

extern "C" {
#include <fs/file.h>
}


class MemoryFS {
public:
    MemoryFS(char* buffer, size_t buffer_size);
    filesystem* get_struct();

    uint32_t get_buffer_size();

    int read(uint32_t size, char *out);

    int write(char *data, uint32_t size);

    int seek(uint32_t offset, FILE_SEEK_MODE seek_mode);

private:
    uint32_t m_buffer_idx;
    char* m_buffer;
    size_t m_buffer_size;
};

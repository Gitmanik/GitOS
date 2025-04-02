//
// Created by Paweł Reich on 2/4/25.
//

#pragma once
#include <stdint-gcc.h>
#include <stddef.h>

extern "C" {
#include <fs/file.h>
}

class PipeFS {
    public:
        PipeFS(size_t buffer_size);
        filesystem* get_struct();

        uint32_t get_buffer_size();

        int read(uint32_t size, char *out);

        int write(const char *data, uint32_t size);

    private:
        char* m_buffer;
        size_t m_buffer_size;
        uint32_t m_buffer_read_idx;
        uint32_t m_buffer_write_idx;
        uint32_t m_count;
};

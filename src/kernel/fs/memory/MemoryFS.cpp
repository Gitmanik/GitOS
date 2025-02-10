//
// Created by Paweł Reich on 2/7/25.
//

#include "MemoryFS.hpp"

extern "C" {
#include <common/assert.h>
#include <common/status.h>
#include <memory/memory.h>
}


int memory_close([[maybe_unused]] void* data) {
    assert_not_reached();
    return 0;
}

void* memory_open(void* data, [[maybe_unused]] struct path_part* path, [[maybe_unused]] FILE_MODE mode) {
    return data;
}

int memory_read([[maybe_unused]] void* private_fs, void* desc, uint32_t size, uint32_t nmemb, char* out) {
    auto* fs = static_cast<MemoryFS *>(desc);
    for (uint32_t i = 0; i < nmemb; i++)
    {
        fs->read(size, out);
        out += size;
    }
    return nmemb;
}

int memory_write([[maybe_unused]] void* private_fs, void* desc, uint32_t size, uint32_t nmemb, char* in) {
    auto* fs = static_cast<MemoryFS *>(desc);
    for (uint32_t i = 0; i < nmemb; i++)
    {
        fs->write(in, size);
        in += size;
    }
    return nmemb;
}

int memory_seek([[maybe_unused]] void* desc, [[maybe_unused]] uint32_t offset, [[maybe_unused]] FILE_SEEK_MODE seek_mode) {

    auto* fs = static_cast<MemoryFS *>(desc);
    return fs->seek(offset, seek_mode);
    return 0;
}

int memory_stat(void* desc, file_stat* stat) {
    auto* fs = static_cast<MemoryFS *>(desc);
    stat->filesize = fs->get_buffer_size();
    stat->flags= FILE_MODE_READ | FILE_MODE_WRITE | FILE_MODE_APPEND;
    return 0;
}

int memory_resolve([[maybe_unused]] struct disk * disk) {
    return 0;
}

MemoryFS::MemoryFS(char* buffer, size_t buffer_size) {
    m_buffer = buffer;
    m_buffer_size = buffer_size;
    m_buffer_idx = 0;
}


filesystem* MemoryFS::get_struct() {
    static filesystem fs = {
        .resolve = memory_resolve,
        .open = memory_open,
        .read = memory_read,
        .seek = memory_seek,
        .stat = memory_stat,
        .close = memory_close,
        .write = memory_write,
        .name = "MemoryFS\0\0\0\0\0\0\0\0\0\0\0",
    };
    return &fs;
}

uint32_t MemoryFS::get_buffer_size() {
    return m_buffer_size;
}

int MemoryFS::read(uint32_t size, char* out) {
    if (m_buffer_idx + size > m_buffer_size) {
        return -EINVARG;
    }
    memcpy(out, m_buffer + m_buffer_idx, size);
    m_buffer_idx += size;
    return 0;
}

int MemoryFS::write(char* data, uint32_t size) {
    if (m_buffer_idx + size > m_buffer_size) {
        return -EINVARG;
    }
    memcpy(m_buffer + m_buffer_idx, data, size);
    m_buffer_idx += size;
    return 0;
}

int MemoryFS::seek(uint32_t offset, FILE_SEEK_MODE seek_mode) {
    if (offset >= m_buffer_size)
        return -EINVARG;

    switch (seek_mode)
    {
        case SEEK_SET:
            m_buffer_idx = offset;
            break;

        case SEEK_CUR:
            if (m_buffer_idx + offset > m_buffer_idx)
                return -EINVARG;

            if ((int32_t) (m_buffer_idx + offset) < 0)
                return -EINVARG;

            m_buffer_idx += offset;
            break;

        case SEEK_END:
            if ((int32_t) (m_buffer_idx - offset) < 0)
                return -EINVARG;

        m_buffer_idx = m_buffer_idx - offset;
        break;
    }
    return ALL_OK;
}
//
// Created by Paweł Reich on 2/4/25.
//

#include "PipeFS.hpp"

#include <common/assert.h>

extern "C" {
#include <memory/memory.h>
#include <memory/heap/kheap.h>
#include <fs/file.h>
}

int pipe_close([[maybe_unused]] void* data) {
    // assert_not_reached();
    return 0;
}

void* pipe_open(void* data, [[maybe_unused]] struct path_part* path, [[maybe_unused]] FILE_MODE mode) {
    return data;
}

int pipe_read([[maybe_unused]] void* private_fs, void* desc, uint32_t size, uint32_t nmemb, char* out) {

    auto* fs = static_cast<PipeFS *>(desc);
    for (uint32_t i = 0; i < nmemb; i++)
    {
        fs->read(size, out);
        out += size;
    }
    return nmemb;

}

int pipe_seek([[maybe_unused]] void* desc, [[maybe_unused]] uint32_t offset, [[maybe_unused]] FILE_SEEK_MODE seek_mode) {
    assert_not_reached();
    return 0;
}

int pipe_stat(void* desc, file_stat* stat) {
    auto* fs = static_cast<PipeFS *>(desc);
    stat->filesize = fs->get_buffer_size();
    stat->flags= FILE_MODE_READ | FILE_MODE_WRITE | FILE_MODE_APPEND;
    return 0;
}

int pipe_resolve([[maybe_unused]] struct disk * disk) {
    return 0;
}

int pipe_write([[maybe_unused]] void * private_fs, void * desc, uint32_t size, uint32_t nmemb, char* in) {
    auto* fs = static_cast<PipeFS *>(desc);
    for (uint32_t i = 0; i < nmemb; i++)
    {
        fs->write(in, size);
        in += size;
    }
    return nmemb;
}
PipeFS::PipeFS(size_t buffer_size) {
    m_buffer = static_cast<char*>(kzalloc(buffer_size));
    m_buffer_size = buffer_size;
    m_buffer_read_idx = 0;
    m_buffer_write_idx = 0;
    m_count = 0;
}

filesystem* PipeFS::get_struct() {
    static filesystem fs = {
        .resolve = pipe_resolve,
        .open = pipe_open,
        .read = pipe_read,
        .seek = pipe_seek,
        .stat = pipe_stat,
        .close = pipe_close,
        .write = pipe_write,
        .name = "PipeFS\0\0\0\0\0\0\0\0\0\0\0\0\0",
    };
    return &fs;
}

uint32_t PipeFS::get_buffer_size() {
    return m_count;
}

int PipeFS::read(uint32_t size, char* out) {
    if (size > m_count) return -1;

    const uint32_t bytes_available = m_count;
    const uint32_t to_read = (size > bytes_available) ? bytes_available : size;

    if (m_buffer_read_idx + to_read <= m_buffer_size) {
        memcpy(out, m_buffer + m_buffer_read_idx, to_read);
        m_buffer_read_idx = (m_buffer_read_idx + to_read) % m_buffer_size;
    } else {
        const uint32_t first_chunk = m_buffer_size - m_buffer_read_idx;
        memcpy(out, m_buffer + m_buffer_read_idx, first_chunk);
        memcpy(out + first_chunk, m_buffer, to_read - first_chunk);
        m_buffer_read_idx = to_read - first_chunk;
    }

    m_count -= to_read;
    return to_read;
}

int PipeFS::write(const char* data, uint32_t size) {
    if (size == 0) return 0;

    if (size + m_count > m_buffer_size) {
        const uint32_t to_discard = size + m_count - m_buffer_size;
        m_buffer_read_idx = (m_buffer_read_idx + to_discard) % m_buffer_size;
        m_count -= to_discard;
    }

    if (m_buffer_write_idx + size <= m_buffer_size) {
        memcpy(m_buffer + m_buffer_write_idx, data, size);
        m_buffer_write_idx = (m_buffer_write_idx + size) % m_buffer_size;
    } else {
        const uint32_t first_chunk = m_buffer_size - m_buffer_write_idx;
        memcpy(m_buffer + m_buffer_write_idx, data, first_chunk);
        memcpy(m_buffer, data + first_chunk, size - first_chunk);
        m_buffer_write_idx = size - first_chunk;
    }

    m_count += size;
    return size;
}
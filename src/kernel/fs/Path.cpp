//
// Created by Pawel Reich on 2/15/25.
//

#include "Path.hpp"

#include <kernel.h>
#include <common/status.h>

extern "C" {
#include <memory/heap/kheap.h>
#include "common/string.h"
}

path_part* Path::parse(const char* path) {
    return parse(path, nullptr);
}

path_part* Path::parse(const char* path, [[maybe_unused]] Path* relative) {

    //TODO: Support relative paths
    if (!valid(path))
        return nullptr;

    auto first = new path_part;
    first->part = static_cast<char *>(kzalloc(MAX_PATH));

    auto current = first;
    const char* current_path_ptr = path;

    int relative_path_idx = 0;
    for (uint32_t path_idx = 0; path_idx < strlen(path); path_idx++) {

        if (path_idx == strlen(path)) {
            break;
        }

        if (current_path_ptr[path_idx] == '/') {
            relative_path_idx = 0;
            current_path_ptr = current_path_ptr + relative_path_idx;
            auto new_part = new path_part;
            new_part->part = static_cast<char *>(kzalloc(MAX_PATH));
            current->next = new_part;
            current = new_part;
        } else {
            current->part[relative_path_idx] = path[path_idx];
            relative_path_idx++;
        }
    }

    return first;
}
bool Path::valid(const char *path) {
    return strlen(path) != 0 && strcmp("0:/", path) == 0;
}

int pathparser_parse(struct path_part **path_root_out, const char *path, [[maybe_unused]] const char *current_dir_path) {
    *path_root_out = Path::parse(path, nullptr);
    if (*path_root_out == nullptr) {
        return -EINVARG;
    }
    return 0;
}

int pathparser_get_drive_number(const char *path) {
    return path[0] - '0';
}

void pathparser_free(struct path_part* path)
{
    path_part* part = path;
    while (part)
    {
        path_part* next_part = part->next;
        kfree(part->part);
        kfree(part);
        part = next_part;
    }
    kfree(path);
}

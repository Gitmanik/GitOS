//
// Created by Pawel Reich on 2/15/25.
//

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PATH 1024
    struct path_part {
        char* part;
        struct path_part* next;
    };
    int pathparser_parse(struct path_part** path_root_out, const char* path, const char* current_dir_path);
    int pathparser_get_drive_number(const char* path);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <stdint-gcc.h>
class Path {

public:
    static path_part* parse(const char *path);
    static path_part* parse(const char *path, Path *relative);

    static bool valid(const char* path);

    static constexpr uint32_t PART_MAX_LEN = 1024;
};
#endif


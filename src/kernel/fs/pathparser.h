#pragma once
#include <stdbool.h>
#define MAX_PATH 108

struct path_part
{
    char* part;
    struct path_part* next;
};

struct path_root
{
    int drive_no;
    struct path_part* first;
};

int pathparser_parse(struct path_root** path_root_out, const char* path, const char* current_dir_path);
void pathparser_free(struct path_root* root);

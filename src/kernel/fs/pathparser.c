#include "pathparser.h"
#include <stdbool.h>
#include "common/string.h"
#include "common/status.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"

/**
 * @brief Ensures beginning of path is structured like <digit>:/
 * 
 * @param path Path to check
 * @return true Path is valid
 * @return false Path is invalid
 */
static bool pathparser_path_valid(const char* path)
{
    int len = strlen(path);

    return (len >= 3 && is_digit(path[0]) && memcmp ((void*)&path[1], ":/", 2) == 0);
}

/**
 * @brief Reads drive number from path and skips past it.
 * 
 * @param path Path
 * @return int Drive number
 */
static int pathparser_get_drive_number(const char** path)
{
    if (!pathparser_path_valid(*path))
        return -EINVARG;

    int drive_no = to_numeric_digit(*path[0]);
    *path += 3; // 0:/... to ...
    return drive_no;
}

/**
 * @brief Allocates struct path_root and sets drive_number
 * 
 * @param drive_number Drive number
 * @return struct path_root* Pointer to path_root struct
 */
static struct path_root* pathparset_create_root(int drive_number)
{
    struct path_root* path_r = kzalloc(sizeof(struct path_root));
    path_r->drive_no = drive_number;
    path_r->first = 0;
    return path_r;
}

/**
 * @brief Reads part between /.../ and skips past it
 * 
 * @param path Path
 * @return const char* part 
 */
static char* pathparser_get_path_part(const char** path)
{
    char* res_path_part = kzalloc(MAX_PATH);
    int chars_parsed = 0;
    while (**path != '/' && **path != 0)
    {
        res_path_part[chars_parsed] = **path;
        *path += 1;
        chars_parsed++;
    }

    if (**path == '/')
        *path +=1;

    if (chars_parsed == 0)
    {
        kfree(res_path_part);
        res_path_part = 0;
    }
    return res_path_part;
}

struct path_part* pathparset_parse_path_part(struct path_part* last_part, const char** path)
{
    char* path_part_str = pathparser_get_path_part(path);
    if (!path_part_str)
        return 0;

    struct path_part* new_part = kzalloc(sizeof(struct path_part));
    new_part->part = path_part_str;
    new_part->next = 0;

    if (last_part)
    {
        last_part->next = new_part;
    }
    return new_part;
}

void pathparser_free(struct path_root* root)
{
    struct path_part* part = root->first;
    while (part)
    {
        struct path_part* next_part = part->next;
        kfree((void*) part->next);
        kfree(part);
        part = next_part;
    }
    kfree(root);
}

int pathparser_parse(struct path_root** path_root_out, const char* path, const char* current_dir_path)
{
    int drive_number = 0;
    const char* tmp_path = path;

    if (strlen(path) > MAX_PATH)
        return -EINVARG;

    drive_number = pathparser_get_drive_number(&tmp_path);
    if (drive_number < 0)
        return -EINVARG;


    *path_root_out = pathparset_create_root(drive_number);
    if (!path_root_out)
        return -ENOMEM;

    struct path_part* first_part = pathparset_parse_path_part(NULL, &tmp_path);
    if (!first_part)
        return -EINVARG;

    ((struct path_root*) *path_root_out)->first = first_part;

    struct path_part* part = pathparset_parse_path_part(first_part, &tmp_path);
    while (part)
    {
        part = pathparset_parse_path_part(part, &tmp_path);
    }

    return 0;
}
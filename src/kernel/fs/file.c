#include "file.h"
#include <stdint.h>
#include <common/assert.h>
#include "kernel.h"
#include "common/string.h"
#include "common/status.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "Path.hpp"

/**
 * @brief Array holding all registered filesystems
 * 
 */
struct filesystem* filesystems[MAX_FILESYSTEMS];

/**
 * @brief Array holding all open file descriptors
 * 
 */
struct file_descriptor* file_descriptors[MAX_FILEDESCRIPTORS];

/**
 * @brief Array holding all mounted files
 *
 */
struct mounted_file** mounted;

/**
 * @brief Returns first free slot for filesystem struct
 * 
 * @return struct filesystem** Pointer to free pointer for filesystem slot
 */
static struct filesystem** fs_get_free_filesystem_slot()
{
    for (int i = 0; i < MAX_FILESYSTEMS; i++)
    {
        if (filesystems[i] == 0)
            return &filesystems[i];
    }

    return 0;
}

/**
 * @brief Allocates and creates new file_descriptor struct
 * 
 * @param desc_out Created file_descriptor struct
 * @return int Status
 */
static int file_new_descriptor(struct file_descriptor** desc_out)
{
    for (int i = 0; i < MAX_FILEDESCRIPTORS; i++)
    {
        if (file_descriptors[i] == 0)
        {
            struct file_descriptor* desc = kzalloc(sizeof(struct file_descriptor));
            //Descriptors start at 1
            desc->index = i+1;
            file_descriptors[i] = desc;
            *desc_out = desc;
            return ALL_OK;
        }
    }
    return -ENOMEM;
}

/**
 * @brief Returns file descriptor struct for given index
 * 
 * @param fd File descriptor index
 * @return struct file_descriptor* Struct for given index
 */
static struct file_descriptor* file_get_descriptor(int fd)
{
    if (fd <= 0 || fd >= MAX_FILEDESCRIPTORS)
        return 0;

    return file_descriptors[fd-1];
}

/**
 * @brief Frees memory allocated by file descriptor
 * 
 * @param desc 
 * @return int 
 */
static int file_free_descriptor(struct file_descriptor* desc)
{
    file_descriptors[desc->index - 1] = 0;
    kfree(desc);
    return ALL_OK;
}


/**
 * @brief Inserts filesystem struct into internal array
 * 
 * @param filesystem 
 */
void fs_insert_filesystem(struct filesystem* filesystem)
{
    struct filesystem** fs;
    if (filesystem == 0)
        kernel_panic("NULL filesystem tried to be registered!");

    fs = fs_get_free_filesystem_slot();
    if (!fs)
    {
        kernel_panic("No slot free in filesystems array");
    }
    *fs = filesystem;
    
    kprintf("Filesystem %s registered\r\n", filesystem->name);
}

/**
 * @brief Initializes internal filesystem arrays
 * 
 */
void fs_init()
{
    memset(filesystems, 0, sizeof(filesystems)); 
    memset(file_descriptors, 0, sizeof(file_descriptors));
    mounted = kzalloc(sizeof(struct mounted_file) * MAX_MOUNTED);
}

/**
 * @brief Resolves filesystem for given disk
 * 
 * @param disk Disk to resolve
 * @return struct filesystem* Resolved filesystem, 0 if not resolved
 */
struct filesystem* fs_resolve(struct disk* disk)
{
    for (int i = 0; i < MAX_FILESYSTEMS; i++)
    {
        if (filesystems[i] != 0 && filesystems[i]->resolve(disk) == ALL_OK)
        {
            kprintf("Resolved filesystem for disk %d: %s\r\n", disk->id, filesystems[i]->name);
            mount("0:", filesystems[i], disk->fs_private);
            return filesystems[i];
        }
    }
    return 0;
}

/**
 * @brief Resolved string filemode to internal
 * 
 * @param str File mode
 * @return FILE_MODE Internal file mode
 */
static FILE_MODE file_get_mode_by_string(const char* str)
{
    if (strncmp(str, "r", 1) == 0)
    {
        return FILE_MODE_READ;
    }
    else if (strncmp(str, "w", 1) == 0)
    {
        return FILE_MODE_WRITE;
    }
    else if (strncmp(str, "a", 1) == 0)
    {
        return FILE_MODE_APPEND;
    }
    else
    {
        return FILE_MODE_INVALID;
    }
}

/**
 * @brief Opens file
 * 
 * @param filename File to open
 * @param str_mode Open mode
 * @return int Status
 */
int fopen(const char* filename, const char* str_mode)
{
    int result = 0;
    struct path_part* root_path;
    result = pathparser_parse(&root_path, filename, NULL);
    if (result < 0)
        return result;

    if (!root_path->next) //Cannot have just root path (without any file)
        return -EINVARG;

    FILE_MODE fmode = file_get_mode_by_string(str_mode);
    if (fmode == FILE_MODE_INVALID)
        return -EINVARG;

    int path_sz = 0;
    struct path_part* part = root_path;
    while (part) {
        part = part->next;
        path_sz++;
    }

    part = root_path;
    struct path_part** parts = kzalloc(path_sz * sizeof(struct path_part*));
    for (int i = 0; i < path_sz; i++) {
        parts[i] = part;
        part = part->next;
    }

    char* reconstructed_path = kzalloc(MAX_PATH);

    kdebug("Trying to search mounted for '%s'", filename);
    int relative_path_to_mounted_idx = path_sz;
    for (; relative_path_to_mounted_idx >= 0; relative_path_to_mounted_idx--) {
        memset(reconstructed_path, 0, MAX_PATH);
        char* ptr = reconstructed_path;
        for (int j = 0; j < relative_path_to_mounted_idx; j++) {
            strcpy(ptr, parts[j]->part);
            ptr += strlen(parts[j]->part);
            if (j < relative_path_to_mounted_idx - 1) {
                *ptr = '/';
                ptr++;
            }
        }
        for (int idx = 0; idx < MAX_MOUNTED; idx++) {
            if (mounted[idx] != 0) {
                if (strlen(reconstructed_path) == strlen(mounted[idx]->filename) && strcmp(reconstructed_path, mounted[idx]->filename) == 0) {
                    kdebug("Found mounted file: '%s' -> '%s', fs root: '%s'", filename, mounted[idx]->fs->name, mounted[idx]->filename);
                    void *descriptor_private_data = mounted[idx]->fs->open(mounted[idx]->data, parts[relative_path_to_mounted_idx], fmode);

                    assert(descriptor_private_data);

                    struct file_descriptor* desc = 0;
                    result = file_new_descriptor(&desc);
                    assert(result == 0);

                    desc->filesystem = mounted[idx]->fs;
                    desc->private_fs_descriptor = descriptor_private_data;
                    desc->private_fs = mounted[idx]->data;
                    result = desc->index;
                    goto out;
                }
            }
        }
    }

    result = -EINVARG;

    out:
    kfree(reconstructed_path);
    kfree(parts);
    pathparser_free(root_path);
    return result;
}

/**
 * @brief Reads from file
 * 
 * @param ptr Output buffer
 * @param size Size in bytes of block
 * @param nmemb Number of blocks to read
 * @param fd File descriptor
 * @return int Status
 */
int fread(void* ptr, uint32_t size, uint32_t nmemb, int fd)
{
    if (size == 0 || nmemb == 0 || fd < 1)
        return -EINVARG;

    struct file_descriptor* desc = file_get_descriptor(fd);

    return desc->filesystem->read(desc->private_fs, desc->private_fs_descriptor, size, nmemb, (char*) ptr);
}

/**
 * @brief Writes to file
 *
 * @param ptr Input buffer
 * @param size Size in bytes of block
 * @param nmemb Number of blocks to read
 * @param fd File descriptor
 * @return int Status
 */
int fwrite(void* ptr, uint32_t size, uint32_t nmemb, int fd)
{
    if (size == 0 || nmemb == 0 || fd < 1)
        return -EINVARG;

    struct file_descriptor* desc = file_get_descriptor(fd);

    return desc->filesystem->write(desc->private_fs, desc->private_fs_descriptor, size, nmemb, (char*) ptr);
}


/**
 * @brief Seeks into file
 * 
 * @param fd File descriptor
 * @param offset Seek offset
 * @param whence File seek mode (SEEK_SET for absolute, SEEK_CUR for relative)
 * @return int Status
 */
int fseek(int fd, int offset, FILE_SEEK_MODE whence)
{
    struct file_descriptor* desc = file_get_descriptor(fd);
    if (!desc)
        return -EIO;

    return desc->filesystem->seek(desc->private_fs_descriptor, offset, whence);
}

/**
 * @brief Returns file status
 * 
 * @param fd File descriptor 
 * @param stat Output file status struct
 * @return int Status
 */
int fstat(int fd, struct file_stat* stat)
{
    struct file_descriptor* desc = file_get_descriptor(fd);
    if (!desc)
        return -EIO;

    return desc->filesystem->stat(desc->private_fs_descriptor, stat);
}

/**
 * @brief Closes file descriptor
 * 
 * @param fd File descriptor
 * @return int Status
 */
int fclose(int fd)
{
    struct file_descriptor* desc = file_get_descriptor(fd);
    if (!desc)
        return -EIO;

    int res = desc->filesystem->close(desc->private_fs_descriptor);
    file_free_descriptor(desc);
    return res;
}

void mount(const char *filename, struct filesystem* fs, void* data) {
    struct mounted_file* mf = kzalloc(sizeof(struct mounted_file));
    mf->fs = fs;
    mf->data = data;
    mf->filename = filename;
    for (int idx = 0; idx < MAX_MOUNTED; idx++) {
        if (mounted[idx] == 0) {
            mounted[idx] = mf;
            kdebug("[FS] Mounted %s pointing to %s at idx %d", filename, fs->name, idx);
            return;
        }
    }
    kernel_panic("No more mount slots");
}

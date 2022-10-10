#include "file.h"
#include <stdint.h>
#include "kernel.h"
#include "common/string.h"
#include "common/status.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "drivers/disk/disk.h"

struct filesystem* filesystems[MAX_FILESYSTEMS];
struct file_descriptor* file_descriptors[MAX_FILEDESCRIPTORS];

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

    return ALL_OK;
}

/**
 * @brief Allocates and creates new file_descriptor struct
 * 
 * @param desc_out Created file_descriptor struct
 * @return int Status
 */
static int file_new_descriptor(struct file_descriptor** desc_out)
{
    for (int i = 0; MAX_FILEDESCRIPTORS; i++)
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
    
    char message[] = "Filesystem %s registered\r\n";
    char buf[sizeof(message) + MAX_FILESYSTEM_NAME];
    kernel_message(ksprintf(buf, message, filesystem->name), GREY);
}

/**
 * @brief Initializes internal filesystem arrays
 * 
 */
void fs_init()
{
    memset(filesystems, 0, sizeof(filesystems)); 
    memset(file_descriptors, 0, sizeof(file_descriptors)); 
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
            return filesystems[i];
        }
    }
    return 0;
}

FILE_MODE file_get_mode_by_string(const char* str)
{
    FILE_MODE mode = FILE_MODE_INVALID;
    if (strncmp(str, "r", 1) == 0)
    {
        mode = FILE_MODE_READ;
    }
    else if (strncmp(str, "w", 1) == 0)
    {
        mode = FILE_MODE_WRITE;
    }
    else if (strncmp(str, "a", 1) == 0)
    {
        mode = FILE_MODE_APPEND;
    }
    return mode;
}

int fopen(const char* filename, const char* str_mode)
{
    int result = 0;
    struct path_root* root_path;
    result = pathparser_parse(&root_path, filename, NULL);
    if (result < 0)
        return result;

    if (!root_path->first) //Cannot have just root path (without any file)
        return -EINVARG;

    struct disk* disk = disk_get(root_path->drive_no);
    if (!disk)
        return -EIO;

    if (!disk->filesystem)
        return -EINVARG;

    FILE_MODE fmode = file_get_mode_by_string(str_mode);
    if (fmode == FILE_MODE_INVALID)
        return -EINVARG;

    void *descriptor_private_data = disk->filesystem->open(disk, root_path->first, fmode);

    if (descriptor_private_data == 0)
        return -EIO;

    struct file_descriptor* desc = 0;
    result = file_new_descriptor(&desc);
    if (result < 0)
        return result;

    desc->filesystem = disk->filesystem;
    desc->private_buffer = descriptor_private_data;
    desc->disk = disk;
    return desc->index;
}

int fread(void* ptr, uint32_t size, uint32_t nmemb, int fd)
{
    if (size == 0 || nmemb == 0 || fd < 1)
        return -EINVARG;

    struct file_descriptor* desc = file_get_descriptor(fd);

    return desc->filesystem->read(desc->disk, desc->private_buffer, size, nmemb, (char*) ptr);
}

int fseek(int fd, int offset, FILE_SEEK_MODE whence)
{
    struct file_descriptor* desc = file_get_descriptor(fd);
    if (!desc)
        return -EIO;

    return desc->filesystem->seek(desc->private_buffer, offset, whence);
}

int fstat(int fd, struct file_stat* stat)
{
    struct file_descriptor* desc = file_get_descriptor(fd);
    if (!desc)
        return -EIO;

    return desc->filesystem->stat(desc->private_buffer, stat);
}

int fclose(int fd)
{
    struct file_descriptor* desc = file_get_descriptor(fd);
    if (!desc)
        return -EIO;

    int res = desc->filesystem->close(desc->private_buffer);
    file_free_descriptor(desc);
    return res;
}
#include "fat16.h"
#include "common/string.h"
#include "common/status.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "kernel.h"
#include "common/assert.h"

/**
 * @brief Describes driver as filesystem struct for generic filesystem driver use
 * 
 */
struct filesystem fat16_fs = 
{
    .resolve = fat16_resolve,
    .open = fat16_open,
    .read = fat16_read,
    .seek = fat16_seek,
    .stat = fat16_stat,
    .close = fat16_close,
    .write = fat16_write,
};

/**
 * @brief Prepares filesystem struct for generic filesystem driver use
 * 
 * @return struct filesystem* 
 */
struct filesystem* fat16_init_filesystem()
{
    strcpy(fat16_fs.name, "FAT16");
    return &fat16_fs;
}

/**
 * @brief Calculates absolute disk position of first file data cluster
 * 
 * @param file File 
 * @return uint32_t Absolute disk position of first data cluster
 */
static uint32_t fat16_get_first_cluster(struct fat_file* file)
{
    return (file->high_16bits_first_cluster << 16) | file->low_16bits_first_cluster;
}

/**
 * @brief Calculates FAT sector to disk sector
 * 
 * @param fs_private Private filesystem data
 * @param cluster FAT Cluster to calculate
 * @return int Disk sector
 */
static int fat16_cluster_to_sector(struct fat_private* fs_private, int cluster)
{
    return fs_private->root_directory.ending_sector_pos + ((cluster - 2) * fs_private->header.primary.sectors_per_cluster);
}

/**
 * @brief Calculates disk sector to absolute position
 * 
 * @param fs_private Private filesystem data
 * @param sector Sector to calculate
 * @return int Absolute disk position
 */
static int fat16_sector_to_absolute(struct fat_private* fs_private, int sector)
{
    return fs_private->partition_offset + sector * fs_private->header.primary.bytes_per_sector;
}

/**
 * @brief Returns FAT table sector position
 * 
 * @param fs_private Private filesystem data
 * @return uint32_t Disk sector
 */
static uint32_t fat16_get_first_fat_sector(struct fat_private* fs_private)
{
    return fs_private->header.primary.reserved_sectors;
}

/**
 * @brief Converts FAT file (directory) name to null-terminated string
 * 
 * @param out Output null-terminated character array
 * @param in Input FAT-coded character array
 * @param max_len Max length of string to convert
 */
static void fat16_to_proper_string(char** out, const char* in, int max_len)
{
    int c = 0;
    while (*in != 0x00 && *in != 0x20 && c < max_len)
    {
        **out = *in;
        *out +=1;
        in +=1;
        c++;
    }

    if (c < max_len)
        **out = 0;
}

/**
 * @brief Convert FAT file (directory) 8.1 name to null-terminated string
 * 
 * @param file File
 * @param out Output null-terminated character array
 */
static void fat16_get_full_relative_filename(struct fat_file* file, char* out)
{
    memset(out, 0x00, 11);
    char* out_tmp = out;
    fat16_to_proper_string(&out_tmp, (const char*) file->filename, sizeof(file->filename));
    if (file->ext[0] != 0x00 && file->ext[0] != 0x20)
    {
        *out_tmp = '.';
        out_tmp++;
        fat16_to_proper_string(&out_tmp, (const char*) file->ext, sizeof(file->ext));
    }
}

/**
 * @brief Counts total items for given directory
 * 
 * @param fs_private Private filesystem data 
 * @param start_sector Sector position of directory
 * @return int Total amount of items
 */
static int fat16_get_total_items_for_directory(struct fat_private* fs_private, int start_sector)
{
    struct fat_file current_item;

    int total = 0;
#if DEBUG_FAT16
    kdebug("Getting total items for directory, sector %d, absolute %d", start_sector, fat16_sector_to_absolute(fs_private, start_sector));
#endif
    diskstreamer_seek(fs_private->directory_stream, fat16_sector_to_absolute(fs_private, start_sector));

    while (1)
    {
        if (diskstreamer_read(fs_private->directory_stream, &current_item, sizeof(current_item)) !=ALL_OK)
            return -EIO;

        if (current_item.filename[0] == 0x00)  
            break;
    
        if (current_item.filename[0] == 0xE5) //Entry free
            continue;

        char filename_buf[11];
        fat16_get_full_relative_filename(&current_item, filename_buf);
#if DEBUG_FAT16
        kdebug("%d %s: %s, size: %d", (current_item.attribute & FAT_FILE_LONGNAME) ? 1 : 0, current_item.attribute & FAT_FILE_SUBDIRECTORY ? "D" : "F", filename_buf, current_item.filesize);
#endif
        total++;
    }

    return total;
}

/**
 * @brief Resolves root directory
 * 
 * @param fs_private Private filesystem data
 * @param directory Output directory struct
 * @return int Status
 */
static int fat16_get_root_directory(struct fat_private* fs_private, struct fat_directory* directory)
{
    struct fat16_header_primary* primary_header = &fs_private->header.primary;
    int root_dir_sector_pos = (primary_header->fat_copies * primary_header->sectors_per_fat) + fat16_get_first_fat_sector(fs_private);
    int root_dir_size = (primary_header->root_dir_entries * sizeof(struct fat_file));
    int total_sectors = root_dir_size / primary_header->bytes_per_sector;

    if (root_dir_size % primary_header->bytes_per_sector)
        total_sectors +=1;
    
    int total_items = fat16_get_total_items_for_directory(fs_private, root_dir_sector_pos);

    if (total_items < 0)
        return total_items;

    struct fat_file* dir = kzalloc(root_dir_size);
    if (!dir)
        return -ENOMEM;

    if (diskstreamer_seek(fs_private->directory_stream, fat16_sector_to_absolute(fs_private, root_dir_sector_pos)) != ALL_OK)
        return -EIO;

    if (diskstreamer_read(fs_private->directory_stream, dir, root_dir_size) != ALL_OK)
        return -EIO;

    directory->item = dir;
    directory->total = total_items;
    directory->sector_pos = root_dir_sector_pos;
    directory->ending_sector_pos = root_dir_sector_pos + (root_dir_size / primary_header->bytes_per_sector);

    return ALL_OK;
}

/**
 * @brief Prepares private filesystem data
 * 
 * @param disk Disk
 * @param fs_private Output private filesystem data
 * @return int Status
 */
static int fat16_init_private(struct disk* disk, struct fat_private* fs_private)
{
    memset(fs_private, 0, sizeof(struct fat_private));
    fs_private->disk = disk;
    fs_private->cluster_read_stream = diskstreamer_new(disk->id);
    fs_private->fat_read_stream     = diskstreamer_new(disk->id);
    fs_private->directory_stream    = diskstreamer_new(disk->id);

    if (!fs_private->cluster_read_stream || !fs_private->fat_read_stream || !fs_private->directory_stream)
        return -EIO;
    
    return ALL_OK;
}

/**
 * @brief Frees memory allocated by private filesystem data
 * 
 * @param fs_private Private filesystem data
 */
static void fat16_free_private(struct fat_private* fs_private)
{
    if (fs_private->cluster_read_stream) diskstreamer_close(fs_private->cluster_read_stream);
    if (fs_private->fat_read_stream)     diskstreamer_close(fs_private->fat_read_stream);
    if (fs_private->directory_stream)    diskstreamer_close(fs_private->directory_stream);
    kfree(fs_private);
}

/**
 * @brief Frees memory allocated by directory struct
 * 
 * @param directory Directory
 */
static void fat16_free_directory(struct fat_directory* directory)
{
    if (!directory)
        return;
    kfree(directory->item);
    kfree(directory);
}

/**
 * @brief Frees memory allocated by item struct
 * 
 * @param item Item
 */
static void fat16_free_fat_item(struct fat_item* item)
{
    if (item->type == FAT_ITEM_TYPE_DIRECTORY)
    {
        fat16_free_directory(item->directory);
    }
    else if (item->type == FAT_ITEM_TYPE_FILE)
    {
        kfree(item->file);
    }
}

/**
 * @brief Frees memory allocated by internal file descriptor struct
 * 
 * @param desc File descriptor
 */
static void fat16_free_file_descriptor(struct fat_file_descriptor* desc)
{
    fat16_free_fat_item(desc->file);
    kfree(desc);
}

/**
 * @brief Clones file item struct
 * 
 * @param item Item to clone
 * @return struct fat_file* Pointer to cloned struct
 */
static struct fat_file* fat16_clone_directory_item(struct fat_file* item)
{
    struct fat_file* item_copy = 0;
    item_copy = kzalloc(sizeof(struct fat_file));
    if (!item_copy)
    {
        return 0;
    }
    memcpy(item_copy, item, sizeof(struct fat_file));
    return item_copy;
}

/**
 * @brief Resolves next cluster of file
 * 
 * @param disk Disk
 * @param fs_private Private filesystem data
 * @param cluster Previous file cluster
 * @return int Next file cluster number (or Status)
 */
static int fat16_get_next_fat_entry(struct fat_private* fs_private, int cluster)
{
    int res = -1;
    struct disk_stream* stream = fs_private->fat_read_stream;
    if (!stream)
        goto out;

    uint32_t fat_table_position = fat16_sector_to_absolute(fs_private, fat16_get_first_fat_sector(fs_private));
    res = diskstreamer_seek(stream, fat_table_position + (cluster * FAT16_FAT_ENTRY_SIZE));
    if (res < 0)
        goto out;

    uint16_t result = 0;
    res = diskstreamer_read(stream, &result, sizeof(result));
    if (res < 0)
        goto out;

    res = result;

    out:
    return res;
}

/**
 * @brief Resolves n-th data cluster from FAT for given starting FAT cluster
 * 
 * @param fs_private Private filesystem data 
 * @param starting_cluster First data cluster
 * @param offset Data offset in bytes
 * @return int Status
 */
static int fat16_get_nth_cluster_from_fat(struct fat_private* fs_private, int starting_cluster, int offset)
{
    int result = 0;
    int size_of_cluster_bytes = fs_private->header.primary.sectors_per_cluster * fs_private->header.primary.bytes_per_sector;
    int cluster_to_use = starting_cluster;
    int clusters_ahead = offset/size_of_cluster_bytes;
    for (int i = 0; i < clusters_ahead; i++)
    {
        int entry = fat16_get_next_fat_entry(fs_private, cluster_to_use);
        if (entry == 0xFFF8 || entry == 0xFFFF)
        {
            //Last entry in the file but size specified otherwise
            result = -EIO;
            goto out;
        }
        if (entry == FAT16_BAD_SECTOR)
        {
            result = -EIO;
            goto out;
        }
        
        // Reserved
        if (entry == 0xFF0 || entry == 0xFF6)
        {
            result = -EIO;
            goto out;
        }
 
        if (entry == 0x00)
        {
            result = -EIO;
            goto out;
        }
        cluster_to_use = entry;
    }

    result = cluster_to_use;

    out:
    return result;
}

/**
 * @brief Reads from disk stream to buffer
 * 
 * @param fs_private Private filesystem data
 * @param stream Disk stream to read from
 * @param cluster Cluster number
 * @param offset Offset from cluster
 * @param total Total bytes to read
 * @param out Output buffer
 * @return int Status
 */
static int fat16_read_cluster(struct fat_private* fs_private, struct disk_stream* stream, int cluster, int offset, int total, void* out)
{
    int result = 0;

    while (total > 0) {
        int size_of_cluster_bytes = fs_private->header.primary.sectors_per_cluster * fs_private->header.primary.bytes_per_sector;
        int cluster_to_use = fat16_get_nth_cluster_from_fat(fs_private, cluster, offset);
        assert(cluster_to_use > 0);

        int offset_from_cluster = offset % size_of_cluster_bytes;

        int starting_sector = fat16_cluster_to_sector(fs_private, cluster_to_use);
        int starting_pos = fat16_sector_to_absolute(fs_private, starting_sector) + offset_from_cluster;
        int total_to_read = total > size_of_cluster_bytes ? size_of_cluster_bytes : total;

    #if DEBUG_FAT16
        kdebug("Reading %d bytes from starting cluster %d, target cluster %d (sector %d), offset from cluster: %d, total: %d, total_to_read: %d", total, cluster, cluster_to_use, starting_sector, offset_from_cluster, total, total_to_read);
    #endif
        result = diskstreamer_seek(stream, starting_pos);
        assert(result == ALL_OK);

        result = diskstreamer_read(stream, out, total_to_read);
        assert(result == ALL_OK);

        total -= total_to_read;
        offset += total_to_read;
        out += total_to_read;
    }

    return 0;
}

/**
 * @brief Reads directory to internal directory struct
 * 
 * @param fs_private Private filesystem data
 * @param item Item to read
 * @return struct fat_directory* 
 */
static struct fat_directory* fat16_load_fat_directory(struct fat_private* fs_private, struct fat_file* item)
{
    int result = 0;

    struct fat_directory* directory = 0;
    if (!(item->attribute & FAT_FILE_SUBDIRECTORY))
    {
        result = -EIO;
        goto out;
    }

    directory = kzalloc(sizeof(struct fat_directory));
    if (!directory)
    {
        result = -ENOMEM;
        goto out;
    }

    int cluster = fat16_get_first_cluster(item);
    int cluster_sector = fat16_cluster_to_sector(fs_private, cluster);
    
    int total_items = fat16_get_total_items_for_directory(fs_private, cluster_sector);
    directory->total = total_items;

    int directory_size = directory->total * sizeof(struct fat_file);
    directory->item = kzalloc(directory_size);

    if (!directory->item)
    {
        result = -ENOMEM;
        goto out;
    }
        

    if (fat16_read_cluster(fs_private, fs_private->cluster_read_stream ,cluster, 0x00, directory_size, directory->item) != ALL_OK)
    {   
        result = -EIO;
        goto out;
    }

    out:
    if (directory)
        if (result != ALL_OK)
        {
            kdebug("Could not read directory! Error code: %d", result);
            fat16_free_directory(directory);
        }

    return directory;
}

/**
 * @brief Allocates new item struct for given item struct
 * 
 * @param fs_private Private filesystem data
 * @param item Item to generate struct for
 * @return struct fat_item* Pointer to allocated struct
 */
static struct fat_item* fat16_new_fat_item_for_directory_item(struct fat_private* fs_private, struct fat_file* item)
{
    struct fat_item* f_item = kzalloc(sizeof(struct fat_item));
    if (!f_item)
        return 0;

    if (item->attribute & FAT_FILE_SUBDIRECTORY)
    {
        f_item->directory = fat16_load_fat_directory(fs_private, item);
        f_item->type = FAT_ITEM_TYPE_DIRECTORY;
    }
    else
    {
        f_item->type = FAT_ITEM_TYPE_FILE;
        f_item->file = fat16_clone_directory_item(item);
    }

    return f_item;
}

/**
 * @brief Finds item in directory
 * 
 * @param fs_private Private filesystem data
 * @param directory Directory
 * @param name File name to find
 * @return struct fat_item* Pointer to allocated file struct
 */
static struct fat_item* fat16_find_item_in_directory(struct fat_private* fs_private, struct fat_directory* directory, const char* name)
{
    struct fat_item* f_item = 0;
    char tmp_filename[11];
    for (int i = 0; i < directory->total; i++)
    { 
        fat16_get_full_relative_filename(&directory->item[i], tmp_filename);
        if (istrncmp(tmp_filename, name, sizeof(tmp_filename)) == 0)
        {
            f_item = fat16_new_fat_item_for_directory_item(fs_private, &directory->item[i]);
        } 
    }
    return f_item;
}

/**
 * @brief Generates directory struct
 * 
 * @param fs_private Private filesystem data 
 * @param path Path to directory
 * @return struct fat_item* Pointer to generated struct
 */
static struct fat_item* fat16_get_directory_entry(struct fat_private* fs_private, struct path_part* path)
{
    struct fat_item* current_item = 0;
    struct fat_item* root_item = fat16_find_item_in_directory(fs_private, &fs_private->root_directory, path->part);

    if (!root_item)
    {
        goto out;
    }

    struct path_part* next_part = path->next;
    current_item = root_item;
    while (next_part != 0)
    {
        if (current_item->type != FAT_ITEM_TYPE_DIRECTORY)
        {
            current_item = 0;
            break;
        }
        struct fat_item* tmp_item = fat16_find_item_in_directory(fs_private, current_item->directory, next_part->part); // Go to next directory
        fat16_free_fat_item(current_item); 
        current_item = tmp_item;
        next_part = next_part->next;
    }

out:
    return current_item;
}

/**
 * @brief FAT16 filesystem resolver
 * 
 * @param disk Disk to resolve
 * @return int Status
 */
int fat16_resolve(struct disk* disk)
{
#if DEBUG_FAT16
    kdebug("fat16_resolve: disk: 0x%p", disk);
#endif
    int result = ALL_OK;

    struct fat_private* fs_private = kzalloc(sizeof(struct fat_private));

    fat16_init_private(disk, fs_private);
    disk->fs_private = fs_private;
    disk->filesystem = &fat16_fs;

    struct disk_stream* stream = diskstreamer_new(disk->id);
    if (!stream)
    {
        result = -ENOMEM;
        goto out;
    }

    uint8_t mbr[512];
    if (diskstreamer_read(stream, mbr, sizeof(mbr)) != ALL_OK) {
        result = -EIO;
        goto out;
    }

    // Verify MBR signature
    if (mbr[510] != 0x55 || mbr[511] != 0xAA) {
        kdebug("Invalid MBR signature (disk %d)", disk->id);
        result = -EIO;
        goto out;
    }

    // Parse partition entries (located at offset 0x1BE)
    struct partition_entry* partitions = (struct partition_entry*)(mbr + 0x1BE);
    uint32_t partition_offset = 0;
    uint32_t partition_number = -1;
    for (uint32_t i = 0; i < 4; i++) {
        if (partitions[i].type == 0x04 || partitions[i].type == 0x06 || partitions[i].type == 0x0E) {
            partition_number = i;
            partition_offset = partitions[i].starting_lba * disk->sector_size;
            break;
        }
    }

    if (partition_offset == 0) {
        kdebug("No FAT16 partition found in MBR (disk %d)", disk->id);
    }

    fs_private->partition_offset = partition_offset;

    if (diskstreamer_seek(stream, partition_offset) != ALL_OK) {
        result = -EIO;
        goto out;
    }
    
    result = diskstreamer_read(stream, &fs_private->header, sizeof(fs_private->header));
    if (result != ALL_OK)
    {
        kdebug("Could not read FAT header from partition in offset %d: %d (disk %d)", partition_offset, result, disk->id);
        result = -EIO;
        goto out;
    }

    if (fs_private->header.shared.extended.signature != 0x29)
    {
        kdebug("FAT Header Signature did not match! Expected 0x29, found 0x%x", fs_private->header.shared.extended.signature);
        result = -EIO;
        goto out;
    }

    result = fat16_get_root_directory(fs_private, &fs_private->root_directory);
    if (result != ALL_OK)
    {
        kdebug("Could not read root directory! Error code: %d", result);
        result = -EIO;
        goto out;
    }

    out:
    if (stream)
        diskstreamer_close(stream);

    if (result < 0)
    {
        fat16_free_private(fs_private);
        disk->fs_private = 0;
    }
    else
    {
        kdebug("fat16_resolve: Found FAT16 partition at partition index: %d", partition_number)
    }

    return result;
}

/**
 * @brief FAT16 Implementation of fopen
 * 
 * @param private_fs Private filesystem data
 * @param path Path to file
 * @param mode Open mode (FILE_MODE_READ for readonly)
 * @return void* Pointer to private file descriptor
 */
void* fat16_open(void* private_fs, struct path_part* path, FILE_MODE mode)
{
#if DEBUG_FAT16
    kdebug("fat16_open: private_fs: 0x%p, path: 0x%p, path->part: %s, mode: %d", private_fs, path, path->part, mode);
#endif
    struct fat_private* fs_private = private_fs;
    if (mode != FILE_MODE_READ)
        return 0;

    struct fat_file_descriptor* descriptor = kzalloc(sizeof(struct fat_file_descriptor));
    if (!descriptor)
        return 0;

    descriptor->file = fat16_get_directory_entry(fs_private, path);
    if (!descriptor->file)
        return 0;

    descriptor->pos = 0;

    return descriptor;
}

/**
 * @brief FAT16 Implementation of fread
 * 
 * @param private_fs Private filesystem data
 * @param desc Internal file descriptor
 * @param size Size of block
 * @param nmemb Number of blocks to read
 * @param out Output buffer
 * @return int Status
 */
int fat16_read(void* private_fs, void* desc, uint32_t size, uint32_t nmemb, char* out)
{
#if DEBUG_FAT16
    kdebug("fat16_read: private_fs: 0x%p, desc: 0x%p, size: %d, nmemb: %d, out: 0x%p", private_fs, desc, size, nmemb, out);
#endif

    struct fat_private* fs_private = private_fs;
    struct fat_file_descriptor* fat_desc = desc;

    struct fat_file* file = fat_desc->file->file;
    int offset = fat_desc->pos;
    for (uint32_t i = 0; i < nmemb; i++)
    {
        int res = fat16_read_cluster(fs_private, fs_private->cluster_read_stream, fat16_get_first_cluster(file), fat_desc->pos, size, out);
        if (res != ALL_OK)
            return res;
        out += size;
        offset += size;
    }
    return nmemb;
}

int fat16_write(void* private_fs, void* desc, uint32_t size, uint32_t nmemb, char* in)
{
#if DEBUG_FAT16
    kdebug("fat16_write: private_fs: 0x%p, desc: 0x%p, size: %d, nmemb: %d, in: 0x%p", private_fs, desc, size, nmemb, in);
#endif

    (void)(private_fs);
    (void)(desc);
    (void)(size);
    (void)(nmemb);
    (void)(in);

    return -ENOTIMPL;
}

/**
 * @brief FAT16 Implementation of fseek
 * 
 * @param desc Internal file descriptor 
 * @param offset Offset
 * @param seek_mode Seek mode (SEEK_SET for absolute, SEEK_CUR for relative)
 * @return int Status
 */
int fat16_seek(void* desc, uint32_t offset, FILE_SEEK_MODE seek_mode)
{
#if DEBUG_FAT16
    kdebug("fat16_seek: desc: 0x%p, offset: %d, seek_mode: %d", desc, offset, seek_mode);
#endif

    struct fat_file_descriptor* fat_desc = desc;

    if (fat_desc->file->type != FAT_ITEM_TYPE_FILE)
        return -EINVARG;

    struct fat_file* item = fat_desc->file->file;
    
    if (offset >= item->filesize)
        return -EINVARG;

    switch (seek_mode)
    {
        case SEEK_SET:
            fat_desc->pos = offset;
            break;

        case SEEK_CUR:
            if (fat_desc->pos + offset > item->filesize)
                return -EINVARG;
            if ((int32_t) (fat_desc->pos + offset) < 0)
                return -EINVARG;
            fat_desc->pos += offset;
            break;
        case SEEK_END:
            if ((int32_t) (item->filesize - offset) < 0)
                return -EINVARG;

            fat_desc->pos = item->filesize - offset;
            break; 
    }
    return ALL_OK;
}

/**
 * @brief FAT16 Implementation of fstat
 * 
 * @param desc Internal file descriptor
 * @param stat Output fstat struct
 * @return int Status
 */
int fat16_stat(void* desc, struct file_stat* stat)
{
#if DEBUG_FAT16
    kdebug("fat16_stat: desc: 0x%p, stat: 0x%p", desc, stat);
#endif

    struct fat_file_descriptor* fat_desc = desc;

    struct fat_item* desc_item = fat_desc->file;
    struct fat_file* file = desc_item->file;

    if (desc_item->type == FAT_ITEM_TYPE_FILE) {
        stat->filesize = file->filesize;
        stat->flags = 0;

        if (file->attribute & FAT_FILE_READONLY)
            stat->flags |= FILE_STAT_READ_ONLY;
    }
    else if (desc_item->type == FAT_ITEM_TYPE_DIRECTORY) {
        stat->filesize = 0;
        stat->flags = FILE_STAT_FOLDER;
    }
    else {
        assert_not_reached();
    }

    return ALL_OK;
} 

/**
 * @brief FAT16 Implementation of fclose
 * 
 * @param desc Internal file descriptor 
 * @return int Status
 */
int fat16_close(void* desc)
{
#if DEBUG_FAT16
    kdebug("fat16_close: desc: 0x%p", desc);
#endif
    struct fat_file_descriptor* fat_desc = desc;
    fat16_free_file_descriptor(fat_desc);
    return ALL_OK;
}

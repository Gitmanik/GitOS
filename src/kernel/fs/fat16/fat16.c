#include "fat16.h"
#include "common/string.h"
#include "common/status.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "kernel.h"

struct filesystem fat16_fs = 
{
    .resolve = fat16_resolve,
    .open = fat16_open,
    .read = fat16_read,
    .seek = fat16_seek,
    .stat = fat16_stat,
    .close = fat16_close
};

static uint32_t fat16_get_first_cluster(struct fat_directory_item* item)
{
    return (item->high_16bits_first_cluster) | item->low_16bits_first_cluster;
}

static int fat16_cluster_to_sector(struct fat_private* private, int cluster_number)
{
    // TODO: Read why - 2 (FAT docs). A: Two first sectors are ignored.
    return private->root_directory.ending_sector_pos + ((cluster_number - 2) * private->header.primary.sectors_per_cluster);
}

static int fat16_sector_to_absolute(struct disk* disk, int sector)
{
    return sector * disk->sector_size;
}

static uint32_t fat16_get_first_fat_sector(struct fat_private* private)
{
    return private->header.primary.reserved_sectors;
}

static void fat16_to_proper_string(char** out, const char* in)
{
    while (*in != 0x00 && *in != 0x20)
    {
        **out = *in;
        *out +=1;
        in++;
    }

    if (*in == 0x20) // If is FAT filename terminator replace with null terminator
    {
        **out = 0x00;
    }
}

static int fat16_get_total_items_for_directory(struct disk* disk, int start_sector)
{
    struct fat_directory_item item;
    struct fat_directory_item empty_item;
    memset(&empty_item, 0, sizeof(empty_item));

    struct fat_private* fat_private = disk->fs_private;

    int i = 0;
    diskstreamer_seek(fat_private->directory_stream, start_sector * disk->sector_size);

    while (1)
    {
        if (diskstreamer_read(fat_private->directory_stream, &item, sizeof(item)) !=ALL_OK)
            return -EIO;


        if (item.filename[0] == 0x00)  
        {
            //All read
            break;
        }

        if (item.filename[0] == 0xE5) //Entry free
            continue;

        kprintf("%s, %s, size: %d\r\n", item.attribute & FAT_FILE_SUBDIRECTORY ? "Dir " : "File", (char*) item.filename, item.filesize);
        i++;
    }

    return i;
}

static int fat16_get_root_directory(struct disk* disk, struct fat_private* fat_private, struct fat_directory* directory)
{
    struct fat16_header_primary* primary_header = &fat_private->header.primary;
    int root_dir_sector_pos = (primary_header->fat_copies * primary_header->sectors_per_fat) + primary_header->reserved_sectors;
    int root_dir_entries = primary_header->root_dir_entries;
    int root_dir_size = (root_dir_entries * sizeof(struct fat_directory_item));
    int total_sectors = root_dir_size / disk->sector_size;

    if (root_dir_size % disk->sector_size) 
        total_sectors +=1;
    
    int total_items = fat16_get_total_items_for_directory(disk, root_dir_sector_pos);

    struct fat_directory_item* dir = kzalloc(root_dir_size);
    if (!dir)
        return -ENOMEM;

    if (diskstreamer_seek(fat_private->directory_stream, fat16_sector_to_absolute(disk, root_dir_sector_pos)) != ALL_OK)
        return -EIO;

    if (diskstreamer_read(fat_private->directory_stream, dir, root_dir_size) != ALL_OK)
        return -EIO;

    directory->item = dir;
    directory->total = total_items;
    directory->sector_pos = root_dir_sector_pos;
    directory->ending_sector_pos = root_dir_sector_pos + (root_dir_size / disk->sector_size);

    return ALL_OK;
}

static int fat16_init_private(struct disk* disk, struct fat_private* private)
{
    memset(private, 0, sizeof(private));
    private->cluster_read_stream = diskstreamer_new(disk->id);
    private->fat_read_stream     = diskstreamer_new(disk->id);
    private->directory_stream    = diskstreamer_new(disk->id);
    return ALL_OK;
}

static void fat16_free_private(struct fat_private* private)
{
    if (private->cluster_read_stream) diskstreamer_close(private->cluster_read_stream);
    if (private->fat_read_stream)     diskstreamer_close(private->fat_read_stream);
    if (private->directory_stream)    diskstreamer_close(private->directory_stream);
    kfree(private);
}

static void fat16_free_directory(struct fat_directory* directory)
{
    if (!directory)
        return;
    kfree(directory->item);
    kfree(directory);
}

static void fat16_fat_item_free(struct fat_item* item)
{
    if (item->type == FAT_ITEM_TYPE_DIRECTORY)
    {
        fat16_free_directory(item->directory);
    }
    else if (item->type == FAT_ITEM_TYPE_FILE)
    {
        kfree(item->item);
    }
    else
    {
        kernel_panic("Tried to free invalid fat_item!");
    }
}

static void fat16_free_file_descriptor(struct fat_file_descriptor* desc)
{
    fat16_fat_item_free(desc->item);
    kfree(desc);
}

static struct fat_directory_item* fat16_clone_directory_item(struct fat_directory_item* item, int size)
{
    struct fat_directory_item* item_copy = 0;
    if (size < sizeof(struct fat_directory_item))
    {
        return 0;
    }
    item_copy = kzalloc(size);
    if (!item_copy)
    {
        return 0;
    }
    memcpy(item_copy, item, size);
    return item_copy;
}

static int fat16_get_fat_entry(struct disk* disk, int cluster)
{
    int result = -1;
    struct fat_private* private = disk->fs_private;
    struct disk_stream* stream = private->fat_read_stream;
    if (!stream)
        goto out;

    uint32_t fat_table_position = fat16_get_first_fat_sector(private) * disk->sector_size;
    result = diskstreamer_seek(stream, fat_table_position + (cluster * FAT16_FAT_ENTRY_SIZE));
    if (result < 0)
        goto out;

    int entry = 0;
    entry = diskstreamer_read(stream, &result, sizeof(result));
    if (entry < 0)
        goto out;

    result = entry;

    out:
    return result;
}

static int fat16_get_cluster_for_offset(struct disk* disk, int starting_cluster, int offset)
{
    int result = 0;
    struct fat_private* private = disk->fs_private;
    int size_of_cluster_bytes = private->header.primary.sectors_per_cluster * disk->sector_size;
    int cluster_to_use = starting_cluster;
    int clusters_ahead = offset/size_of_cluster_bytes;
    for (int i = 0; i < clusters_ahead; i++)
    {
        int entry = fat16_get_fat_entry(disk, cluster_to_use);
        if (entry == 0xFF8 || entry == 0xFFF)
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
        
        // Reserve)
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

static int fat16_read_internal_from_stream(struct disk* disk, struct disk_stream* stream, int cluster, int offset, int total, void* out)
{
    int result = 0;
    struct fat_private* private = disk->fs_private;
    int size_of_cluster_bytes = private->header.primary.sectors_per_cluster * disk->sector_size;
    int cluster_to_use = fat16_get_cluster_for_offset(disk, cluster, offset);
    if (cluster_to_use < 0)
    {
        result = cluster_to_use;
        goto out;
    }

    int offset_from_cluster = offset % size_of_cluster_bytes;

    int starting_sector = fat16_cluster_to_sector(private, cluster_to_use);
    int starting_pos = (starting_sector * disk->sector_size) + offset_from_cluster;
    int total_to_read = total > size_of_cluster_bytes ? size_of_cluster_bytes : total;

    result = diskstreamer_seek(stream, starting_pos);
    if (result != ALL_OK)
    {
        goto out;
    }    

    result = diskstreamer_read(stream, out, total_to_read);
    if (result != ALL_OK)
    {
        goto out;
    }
    total -= total_to_read;

    if (total > 0)
    {
        result = fat16_read_internal_from_stream(disk, stream, cluster, offset+total_to_read, total, out + total_to_read);
    }

    out:
    return result;
}

static int fat16_read_internal(struct disk* disk, int starting_cluster, int offset, int total, void* out)
{
    struct fat_private* fs_private = disk->fs_private;
    struct disk_stream* stream = fs_private->cluster_read_stream;
    return fat16_read_internal_from_stream(disk, stream, starting_cluster, offset, total, out);
}

void fat16_get_full_relative_filename(struct fat_directory_item* item, char* out, int max_len)
{
    memset(out, 0x00, max_len);
    char* out_tmp = out;
    fat16_to_proper_string(&out_tmp, (const char*) item->filename);
    if (item->ext[0] != 0x00 && item->ext[0] != 0x20)
    {
        *out_tmp = '.';
        out_tmp++;
        fat16_to_proper_string(&out_tmp, (const char*) item->ext);
    }
}

struct fat_directory* fat16_load_fat_directory(struct disk* disk, struct fat_directory_item* item)
{
    int result = 0;

    struct fat_directory* directory = 0;
    struct fat_private* fat_private=  disk->fs_private;
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
    int cluster_sector = fat16_cluster_to_sector(fat_private, cluster);
    
    int total_items = fat16_get_total_items_for_directory(disk, cluster_sector);
    directory->total = total_items;

    int directory_size = directory->total * sizeof(struct fat_directory_item);
    directory->item = kzalloc(directory_size);

    if (!directory->item)
    {
        result = -ENOMEM;
        goto out;
    }
        

    if (fat16_read_internal(disk, cluster, 0x00, directory_size, directory->item) != ALL_OK)
    {   
        result = -EIO;
        goto out;
    }

    out:
    if (directory)
        if (result != ALL_OK)
            fat16_free_directory(directory);

    return directory;
}

struct fat_item* fat16_new_fat_item_for_directory_item(struct disk* disk, struct fat_directory_item* item)
{
    struct fat_item* f_item = kzalloc(sizeof(struct fat_item));
    if (!f_item)
        return 0;

    if (item->attribute & FAT_FILE_SUBDIRECTORY)
    {
        f_item->directory = fat16_load_fat_directory(disk, item);
        f_item->type = FAT_ITEM_TYPE_DIRECTORY;
    }
    else
    {
        f_item->type = FAT_ITEM_TYPE_FILE;
        f_item->item = fat16_clone_directory_item(item, sizeof(struct fat_directory_item));
    }

    return f_item;
}

struct fat_item* fat16_find_item_in_directory(struct disk* disk, struct fat_directory* directory, const char* name)
{
    struct fat_item* f_item = 0;
    char tmp_filename[MAX_PATH];
    for (int i = 0; i < directory->total; i++)
    { 
        fat16_get_full_relative_filename(&directory->item[i], tmp_filename, sizeof(tmp_filename));
        if (istrncmp(tmp_filename, name, sizeof(tmp_filename)) == 0)
        {
            f_item = fat16_new_fat_item_for_directory_item(disk, &directory->item[i]);
        } 
    }
    return f_item;
}

struct fat_item* fat16_get_directory_entry(struct disk* disk, struct path_part* path)
{
    struct fat_private* fat_private = disk->fs_private;
    struct fat_item* current_item = 0;
    struct fat_item* root_item = fat16_find_item_in_directory(disk, &fat_private->root_directory, path->part);

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
        struct fat_item* tmp_item = fat16_find_item_in_directory(disk, current_item->directory, next_part->part); // Go to next directory
        fat16_fat_item_free(current_item); 
        current_item = tmp_item;
        next_part = next_part->next;
    }

out:
    return current_item;
}

struct filesystem* fat16_init()
{
    strcpy(fat16_fs.name, "FAT16");
    return &fat16_fs;
}

void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode)
{
    if (mode != FILE_MODE_READ) //TODO: Todo writing
    {
        BochsBreak();
        return -EINVARG;
    }

    struct fat_file_descriptor* descriptor = kzalloc(sizeof(struct fat_file_descriptor));
    if (!descriptor)
    {
        BochsBreak();
        return -ENOMEM;
    }

    descriptor->item = fat16_get_directory_entry(disk, path);
    if (!descriptor->item)
    {
        BochsBreak();
        return -EIO;
    }

    descriptor->pos = 0;

    return descriptor;
}

int fat16_resolve(struct disk* disk)
{
    int result = ALL_OK;

    struct fat_private* fat_private = kzalloc(sizeof(struct fat_private));

    fat16_init_private(disk, fat_private);
    disk->fs_private = fat_private;
    disk->filesystem = &fat16_fs;

    struct disk_stream* stream = diskstreamer_new(disk->id);
    if (!stream)
    {
        result = -ENOMEM;
        goto out;
    }
    
    if (diskstreamer_read(stream, &fat_private->header, sizeof(fat_private->header)))
    {
        result = -EIO;
        goto out;
    }

    if (fat_private->header.shared.extended.signature != 0x29)
    {
        kprintf("FAT Header Signature did not match! Expected 0x29, found 0x%x\r\n", fat_private->header.shared.extended.signature);
        result = -EIO;
        goto out;
    }

    if (fat16_get_root_directory(disk, fat_private, &fat_private->root_directory) != ALL_OK)
    {
        kprintf("Could not read root directory!\r\n");
        result = -EIO;
        goto out;
    }

    out:
    if (stream)
        diskstreamer_close(stream);

    if (result < 0)
    {
        fat16_free_private(fat_private);
        disk->fs_private = 0;
    }

    return result;
}

int fat16_read(struct disk* disk, void* descriptor_buffer, uint32_t size, uint32_t nmemb, char* out)
{
    struct fat_file_descriptor* fat_desc = descriptor_buffer;
    struct fat_directory_item* item = fat_desc->item->item;
    int offset = fat_desc->pos;
    for (uint32_t i = 0; i < nmemb; i++)
    {
        int res = fat16_read_internal(disk, fat16_get_first_cluster(item), offset, size, out);
        if (res != ALL_OK)
            return res;
        out += size;
        offset += size;
    }
    return nmemb;
}

int fat16_seek(void* private_buffer, uint32_t offset, FILE_SEEK_MODE seek_mode)
{
    struct fat_file_descriptor* fat_desc = private_buffer;

    if (fat_desc->item->type != FAT_ITEM_TYPE_FILE)
        return -EINVARG;

    struct fat_directory_item* item = fat_desc->item->item;
    
    if (offset >= item->filesize)
        return -EINVARG;

    switch (seek_mode)
    {
        case SEEK_SET:
            fat_desc->pos = offset;
            break;

        case SEEK_CUR:
            fat_desc->pos += offset;
            break;
        case SEEK_END:
            return -ENOTIMPL; 
    }
    return ALL_OK;
}

int fat16_stat(void* private, struct file_stat* stat)
{
    struct fat_file_descriptor* fat_desc = private;
    struct fat_item* desc_item = fat_desc->item;
    if (desc_item->type != FAT_ITEM_TYPE_FILE)
        return -ENOTIMPL;
    
    struct fat_directory_item* item = desc_item->item;

    stat->filesize = item->filesize;
    stat->flags = 0;

    if (item->attribute & FAT_FILE_READONLY)
        stat->flags |= FILE_STAT_READ_ONLY;

    return ALL_OK;
} 

int fat16_close(void* private)
{
    fat16_free_file_descriptor((struct fat_file_descriptor*) private);
    return ALL_OK;
}
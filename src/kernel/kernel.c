#include "kernel.h"
#include <stdint.h>
#include <stdarg.h>
#include "idt/idt.h"
#include "drivers/text_mode/text_mode.h"
#include "drivers/serial/serial.h"
#include "drivers/pic/pic.h"
#include "common/string.h"
#include "memory/bios_memory_map.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "common/io.h"
#include "memory/paging/paging.h"
#include "drivers/disk/disk.h"
#include "drivers/disk/disk_streamer.h"
#include "fs/pathparser.h"
#include "fs/file.h"
#include "fs/fat16/fat16.h"
#include "gdt/gdt.h"

static struct paging_chunk *kernel_paging_chunk;

struct gdt gdt_real[TOTAL_GDT_SEGMENTS];
struct gdt_structured gdt_structured[TOTAL_GDT_SEGMENTS] = {
    {.base = 0x00, .limit = 0x00, .type = 0x00},       // NULL
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0x9A}, // KERNEL CODE
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0x92}, // KERNEL DATA
};

/**
 * @brief Temporary int 0x0 handler
 *
 */
void divide_by_zero()
{
    kernel_panic("Division by zero error!");
}

/**
 * @brief Located in idt_handler.asm entry point for interrupt 0x0
 *
 */

extern void int21h();
/**
 * @brief Temporary int 0x21 handler
 *
 */
void int21h_handler()
{
    char scan_code = inb(0x60);
    tm_PrintChar(scan_code);
    pic_EOI(0);
}

/**
 * @brief Kernel C entry point
 *
 */
void kernel_main()
{
    int res = 0;

    tm_ClearScreen();
    tm_SetColor(GREY);
    res = ser_Init(COM1, 1);
    if (res < 0)
    {
        kernel_panic("Could not initialize Serial port!");
    }

    kprintf("GitOS - operating system as exercise. Pawel Reich 2022\r\n");

    // Initialize GDT
    kprintf("Initializing GDT..");
    memset(gdt_real, 0, sizeof(gdt_real));
    gdt_structured_to_gdt(gdt_real, gdt_structured, TOTAL_GDT_SEGMENTS);
    gdt_load(gdt_real, sizeof(gdt_real));
    kprintf("OK\r\n");
    //

    // Initialize IDT
    kprintf("Initializing IDT..");
    idt_Init();
    idt_SetDescriptor(0, divide_by_zero);
    idt_SetDescriptor(0x21, int21h);
    idt_Load();
    kprintf("OK\r\n");
    //

    // ksprintf test
    kprintf("kprintf test: %p %x %i %s %c %ld %%\r\n", bios_memory_map, 0x41424344, -1234, "gitmanik.dev", 'X', __LONG_MAX__);
    kdebug("kdebug test: %p %x %i %s %c %ld %%\r\n", bios_memory_map, 0x41424344, -1234, "gitmanik.dev", 'X', __LONG_MAX__);
    //

    // Finding biggest usable memory chunk to use as heap
    memory_map_entry heap_entry;
    int idx = 0;

    kprintf("Usable memory map:\r\n");
    while (bios_memory_map[idx].length_in_bytes > 0)
    {
        if (bios_memory_map[idx].type != 1)
            goto skip;

        kprintf("0x%p -> 0x%p, Size: %ldKB\r\n",
                (long)bios_memory_map[idx].base_address,
                (long)bios_memory_map[idx].base_address + (long)bios_memory_map[idx].length_in_bytes,
                (long)bios_memory_map[idx].length_in_bytes / 1024);

        if (bios_memory_map[idx].length_in_bytes > heap_entry.length_in_bytes)
            heap_entry = bios_memory_map[idx];

    skip:
        idx++;
    }

    // Kernel is located at 0x100000
    if (heap_entry.length_in_bytes < 0x100000)
    {
        kernel_panic("Not enough memory to place kernel heap!");
    }
    heap_entry.base_address += 0x100000;
    heap_entry.length_in_bytes -= 0x100000;

    kprintf("Heap address: 0x%p, Size: %ldKB\r\n", (long)heap_entry.base_address, (long)heap_entry.length_in_bytes / 1024);

    res = kheap_init((void *)(uint32_t)heap_entry.base_address, heap_entry.length_in_bytes);
    if (res < 0)
    {
        kernel_panic("Failed to create heap!");
    }
    //

    kprintf("Enabling paging..");
    kernel_paging_chunk = paging_new_directory(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    paging_switch(paging_get_directory(kernel_paging_chunk));
    paging_enable();
    kprintf("OK\r\n");

    // Paging test
    kprintf("Setting up paging..");
    char *ptr_real = kzalloc(4096);
    res = paging_set_page(paging_get_directory(kernel_paging_chunk), (void *)0x1000, (uint32_t)ptr_real | PAGING_ACCESS_FROM_ALL | PAGING_IS_PRESENT | PAGING_IS_WRITEABLE);
    if (res < 0)
    {
        kernel_panic("Could not sect page!");
    }
    char *ptr_virt = (char *)0x1000;

    ptr_virt[0] = 'O';
    ptr_virt[1] = 'K';

    kprintf("OK\r\n", LIGHT_GREEN);
    kprintf("Paging self-test: 0x%p:'%s' 0x%p:'%s'", (uint32_t)ptr_real, ptr_real, (uint32_t)ptr_virt, ptr_virt);
    if (memcmp(ptr_real, ptr_virt, 2) != 0)
    {
        kernel_panic("\r\nPaging self-test unsuccessful!");
    }
    kprintf("OK\r\n");
    kfree(ptr_real);
    //

    // Remap PIC
    kprintf("Remapping PIC..");

    pic_Remap(0x20, 0x28);

    kprintf(" OK\r\n");
    //

    //
    asm("sti");

    // Initializing FS
    fs_init();
    fs_insert_filesystem(fat16_init_filesystem());
    disk_search_and_init();
    //

    // Read test file
    char *file_to_open = "0:/DIR/FILE.TXT";
    kprintf("Opening \"%s\"..", file_to_open);
    int fd = fopen(file_to_open, "r");
    if (fd < 1)
    {
        kernel_panic("Could not open %s: %d", file_to_open, fd);
    }
    else
    {
        struct file_stat *stat = 0;
        res = fstat(fd, stat);

        if (res < 0)
        {
            kernel_panic("fstat error: %d", res);
        }

        kprintf("File size: %d, Flags: %d\r\n", stat->filesize, stat->flags);

        char *file_content = kzalloc(stat->filesize);

        if (stat->filesize > 22)
        {
            res = fseek(fd, 22, SEEK_SET);
            if (res < 0)
                kernel_panic("fseek error");
        }
        int res = fread(file_content, stat->filesize, 1, fd);
        if (res < 0)
            kernel_panic("fread error");
        kprintf(file_content);
        kprintf("\r\nOK\r\n");

        kfree(file_content);
        fclose(fd);
    }
    //

    tm_SetColor(LIGHT_PURPLE);
    while (1)
    {
        while (!ser_Received(COM1))
            ;
        char c = ser_ReadChar(COM1);
        tm_PrintChar(c);
        if (c == '\r')
            tm_PrintChar('\n');
    }

    kernel_halt();
}

/**
 * @brief (Temporary) Prints kernel_message and halts the kernel.
 *
 * @param fmt Reason of the panic
 * @param ... Arguments
 */
void kernel_panic(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char internal_buf[1024];
    memset(internal_buf, 0, sizeof(internal_buf));
    kvsprintf(internal_buf, fmt, args);

    ser_PrintString(COM1, "Kernel panic!\r\n");
    ser_PrintString(COM1, internal_buf);

    tm_PrintStringColor("Kernel panic!\r\n", LIGHT_RED);
    tm_PrintStringColor(internal_buf, LIGHT_RED);
    kernel_halt();
}

void kernel_halt()
{
    for (;;)
        ;
}

/**
 * @brief Prints to kernel debug channels. Max length of processed message is 1024 characters.
 *
 * @param fmt Message to format and print.
 * @param ... Arguments
 */
void kprintf(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char internal_buf[1024];
    memset(internal_buf, 0, sizeof(internal_buf));

    kvsprintf(internal_buf, fmt, args);
    ser_PrintString(COM1, internal_buf);

    enum TEXT_MODE_COLORS x = tm_GetColor();
    tm_SetColor(GREY);
    tm_PrintString(internal_buf);
    tm_SetColor(x);
}
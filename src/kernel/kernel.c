#include "kernel.h"
#include <stdint.h>
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

static struct paging_chunk* kernel_paging_chunk;

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
    char buf[128];

    tm_ClearScreen();
    tm_SetColor(GREY);
    res = ser_Init(COM1, 1);
    if (res < 0)
    {
        kernel_panic("Panic: Could not initialize Serial port!");
    }
    
    kernel_message("GitOS - operating system as exercise. Pawel Reich 2022\r\n", GREY);

    //Initialize IDT
    kernel_message("Initializing IDT..", GREY);
    idt_Init();
    idt_SetDescriptor(0, divide_by_zero);
    idt_SetDescriptor(0x21, int21h);
    idt_Load();
    kernel_message("OK\r\n",LIGHT_GREEN);
    //

    //ksprintf test
    memset(buf, 0, 128);
    kernel_message(ksprintf(buf, "ksprintf test: %p %x %i %s %c %ld %%\r\n", bios_memory_map, 0x41424344, -1234, "gitmanik.dev", 'X',  __LONG_MAX__), CYAN);
    

    //Finding biggest usable memory chunk to use as heap
    memory_map_entry heap_entry;
    int idx = 0;

    kernel_message("Usable memory map:\r\n",GREY);
    while (bios_memory_map[idx].length_in_bytes > 0)
    {
        if ( bios_memory_map[idx].type != 1)
            goto skip;

        memset(buf, 0, 128);
        kernel_message(ksprintf(buf, "0x%p -> 0x%p, Size: %ldKB\r\n", 
                    (long) bios_memory_map[idx].base_address, 
                    (long) bios_memory_map[idx].base_address + (long) bios_memory_map[idx].length_in_bytes, 
                    (long) bios_memory_map[idx].length_in_bytes / 1024), 
                    YELLOW);

        if (bios_memory_map[idx].length_in_bytes > heap_entry.length_in_bytes)
            heap_entry = bios_memory_map[idx];

        skip:
        idx++;
    }

    //Kernel is located at 0x100000
    if (heap_entry.length_in_bytes < 0x100000)
    {
        kernel_panic("Panic: Not enough memory to place kernel heap!");
    }
    heap_entry.base_address += 0x100000;
    heap_entry.length_in_bytes -= 0x100000;
    
    memset(buf, 0, 128);
    kernel_message(ksprintf(buf, "Heap address: 0x%p, Size: %ldKB\r\n", (long) heap_entry.base_address, (long) heap_entry.length_in_bytes / 1024), GREY);

    res = kheap_init((void*) (uint32_t) heap_entry.base_address, heap_entry.length_in_bytes);
    if (res < 0)
    {
        kernel_panic("Panic: Failed to create heap!");
    }
    //


    kernel_message("Enabling paging..", GREY);
    kernel_paging_chunk = paging_new_directory(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    paging_switch(paging_get_directory(kernel_paging_chunk));
    paging_enable();
    kernel_message("OK\r\n", LIGHT_GREEN);

    //Paging test
    kernel_message("Setting up paging..", GREY);
    char* ptr_real = kzalloc(4096);
    res = paging_set_page(paging_get_directory(kernel_paging_chunk), (void*) 0x1000, (uint32_t) ptr_real | PAGING_ACCESS_FROM_ALL | PAGING_IS_PRESENT | PAGING_IS_WRITEABLE);
    if (res < 0)
    {
        kernel_panic("Panic: Could not sect page!");
    }
    char* ptr_virt = (char*) 0x1000;

    ptr_virt[0] = 'O';
    ptr_virt[1] = 'K';
    
    kernel_message("OK\r\n", LIGHT_GREEN);
    memset(buf, 0, 128);
    kernel_message(ksprintf(buf, "Paging self-test: 0x%p:'%s' 0x%p:'%s'", (uint32_t)ptr_real, ptr_real, (uint32_t) ptr_virt, ptr_virt), GREY);
    if (memcmp(ptr_real, ptr_virt, 2) != 0)
    {
        kernel_panic("\r\nPanic: Paging self-test unsuccessful!");
    }
    kernel_message(" OK\r\n", LIGHT_GREEN);
    kfree(ptr_real);
    //

    //Remap PIC
    kernel_message("Remapping PIC..", GREY);

    pic_Remap(0x20, 0x28);
    
    kernel_message("OK\r\n",LIGHT_GREEN);
    //

    //
    asm("sti");

    //Initializing FS
    fs_init();
    fs_insert_filesystem(fat16_init());
    disk_search_and_init();
    //

    //Read test file
    kernel_message("Opening \"0:/HELLO.TXT\"..", GREY);
    int fd = fopen("0:/HELLO.TXT", "r");
    if (!fd)
    {
        kernel_panic("Could not open 0:/HELLO.TXT");
    }
    else
    {
        char file_content[512];
        int res = fread(file_content, 512, 1, fd);
        if (res < 0)
            kernel_panic("fread error");
        kernel_message(file_content, LIGHT_CYAN);
        kernel_message("\r\nOK\r\n", LIGHT_GREEN);
    }
    //


    tm_SetColor(LIGHT_PURPLE);
    while (1)
    {
        while (!ser_Received(COM1));
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
 * @param message Reason of the panic
 */
void kernel_panic(char* message)
{
    kernel_message("Kernel panic!\r\n", LIGHT_RED);
    kernel_message(message, LIGHT_RED);
    kernel_halt();
}

void kernel_halt()
{
    for (;;);
}

/**
 * @brief Prints message onto Framebuffer and Serial
 * 
 * @param message Message
 * @param col Color
 */
void kernel_message(char* message, enum TEXT_MODE_COLORS col)
{
    enum TEXT_MODE_COLORS x = tm_GetColor();

    tm_SetColor(col);
    tm_PrintString(message);
    tm_SetColor(x);

    ser_PrintString(COM1, message);
}

void kernel_debug(char* message)
{
    enum TEXT_MODE_COLORS x = tm_GetColor();
    tm_SetColor(GREY);
    tm_PrintString(message);
    tm_PrintString("\r\n");
    tm_SetColor(x);

    ser_PrintString(COM1, message);
    ser_PrintString(COM1, "\r\n");
}
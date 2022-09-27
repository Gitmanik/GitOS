#include "kernel.h"
#include <stdint.h>
#include "idt/idt.h"
#include "drivers/text_mode/text_mode.h"
#include "drivers/serial/serial.h"
#include "drivers/pic/pic.h"
#include "common/string.h"
#include "memory/bios_memory_map.h"
#include "memory/heap/kheap.h"

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
    kernel_message("Keyboard pressed!", GREY);
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
    kernel_message("GitOS - operating system as exercise. Pawel Reich 2022\r\n", GREY);

    res = ser_Init(COM1, 1);
    if (res < 0)
    {
        kernel_panic("Panic: Could not initialize Serial port!");
    }
    
    kernel_message("Usable memory map:\r\n",GREY);

    //Finding biggest usable memory chunk to use as heap
    memory_map_entry heap_entry;
    int idx = 0;
    while (bios_memory_map[idx].length_in_bytes > 0)
    {
        if ( bios_memory_map[idx].type != 1)
            goto skip;
        char buf[16];
        kernel_message("0x", YELLOW);
        ltoa(bios_memory_map[idx].base_address, buf, 16);
        kernel_message(buf, YELLOW);
        kernel_message(" -> ", YELLOW);

        kernel_message("0x", YELLOW);
        ltoa(bios_memory_map[idx].base_address + bios_memory_map[idx].length_in_bytes, buf, 16);
        kernel_message(buf, YELLOW);
        kernel_message(" Size: ", YELLOW);

        itoa(bios_memory_map[idx].length_in_bytes, buf, 10);
        kernel_message(buf, YELLOW);
        kernel_message("\r\n", YELLOW);
        if (bios_memory_map[idx].length_in_bytes > heap_entry.length_in_bytes)
            heap_entry = bios_memory_map[idx];

        skip:
        idx++;
    }

    if (heap_entry.length_in_bytes < 0x100000)
    {
        kernel_message("Not enough memory to place kernel heap!", RED);
        kernel_halt();
    }
    heap_entry.base_address += 0x100000;
    heap_entry.length_in_bytes -= 0x100000;

    char buf[16];
    kernel_message("\r\nHeap address: 0x", GREY);
    ltoa(heap_entry.base_address, buf, 16);
    kernel_message(buf, GREY);
    kernel_message("\r\nHeap size: ", GREY);
    ltoa(heap_entry.length_in_bytes / 1024, buf, 10);
    kernel_message(buf, GREY);
    kernel_message("KB\r\n", GREY);

    res = kheap_init((void*) (uint32_t) heap_entry.base_address, heap_entry.length_in_bytes);
    if (res < 0)
    {
        kernel_panic("Panic: Failed to create heap!");
    }
    kernel_message("Remapping PIC..", GREY);

    pic_Remap(0x20, 0x28);
    asm("sti");
    kernel_message("OK\r\n",LIGHT_GREEN);

    kernel_message("Initializing IDT..", GREY);

    idt_Init();
    idt_SetDescriptor(0, divide_by_zero);
    idt_SetDescriptor(0x21, int21h);
    idt_Load();

    kernel_message("OK\r\n",LIGHT_GREEN);

    tm_SetColor(LIGHT_PURPLE);
    while (1)
    {
        while (!ser_IsAvailable(COM1));
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
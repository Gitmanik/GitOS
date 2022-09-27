#include "kernel.h"
#include <stdint.h>
#include "idt/idt.h"
#include "drivers/text_mode/text_mode.h"
#include "drivers/serial/serial.h"
#include "drivers/pic/pic.h"
#include "common/string.h"
#include "memory/bios_memory_map.h"

void divide_by_zero()
{
    tm_PrintString("Division by zero!");
    for (;;);
}

extern void int21h(); 
void int21h_handler()
{
    kernel_message("Keyboard pressed!", GREY);
    pic_EOI(0);
}

void kernel_main()
{
    ser_Init(COM1, 1);
    tm_ClearScreen();

    kernel_message("GitOS - operating system as exercise. Pawel Reich 2022\r\n", GREY);

    kernel_message("Usable memory map:\r\n",GREY);

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

    ser_PrintChar(bios_memory_map[0].type, YELLOW);

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

    while (1)
    {
        while (!ser_IsAvailable(COM1));
        char c = ser_ReadChar(COM1);
        tm_PrintChar(c, LIGHT_PURPLE);
        if (c == '\r')
            tm_PrintChar('\n', LIGHT_PURPLE);
    }

    kernel_halt();
}

void kernel_halt()
{
    for (;;);
}

void kernel_message(char* message, enum TEXT_MODE_COLORS col)
{
    enum TEXT_MODE_COLORS x = tm_GetColor();

    tm_SetColor(col);
    tm_PrintString(message);
    tm_SetColor(x);

    ser_PrintString(COM1, message);
}
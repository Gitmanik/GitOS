#include "ps2keyboard.h"
#include <stdint.h>
#include <stddef.h>
#include "keyboard/keyboard.h"
#include "common/io.h"
#include "kernel.h"
#include "idt/idt.h"
#include "task/task.h"
#include "kernel.h"
#include "drivers/pic/pic.h"

static uint8_t ps2keyboard_scan_set_1[] = {
    0x00, 0x1B, '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '0', '-', '=',
    0x08, '\t', 'Q', 'W', 'E', 'R', 'T',
    'Y', 'U', 'I', 'O', 'P', '[', ']',
    0x0d, 0x00, 'A', 'S', 'D', 'F', 'G',
    'H', 'J', 'K', 'L', ';', '\'', '`', 
    0x00, '\\', 'Z', 'X', 'C', 'V', 'B',
    'N', 'M', ',', '.', '/', 0x00, '*',
    0x00, 0x20, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, '7', '8', '9', '-', '4', '5',
    '6', '+', '1', '2', '3', '0', '.'
};
int ps2keyboard_setup();

struct keyboard ps2keyboard = {
    .name = "PS2 Keyboard",
    .init = ps2keyboard_setup
};

struct keyboard* ps2keyboard_init()
{
    return &ps2keyboard;
}

void ps2keyboard_irq_handler();

int ps2keyboard_setup()
{
    kprintf("Initializing PS2 Keyboard driver\r\n");
    outb(0x64, 0xAE); // Enable first PS/2 port

    idt_SetHandler(0x21, ps2keyboard_irq_handler);

    // TODO: Self-test

    return 0;
}

[[maybe_unused]] static char ps2keyboard_scancode_to_char(uint8_t scancode)
{
    // TODO: Handle other scan sets
    size_t size_of_set = sizeof(ps2keyboard_scan_set_1) / sizeof(uint8_t);

    if (scancode > size_of_set)
    {
        return 0;
    }

    // TODO: Handle Caps Lock

    return ps2keyboard_scan_set_1[scancode];
}

void ps2keyboard_irq_handler()
{
    kernel_page();
    
    pic_EOI(0);

    uint8_t scancode = inb(0x60);
    inb(0x60); // Ignore next byte

    if (scancode & 0x80) //release
    {
        return;
    }

    uint8_t c = ps2keyboard_scancode_to_char(scancode);
    if (c != 0)
    {
        keyboard_push(c);
    }
    task_page();
}

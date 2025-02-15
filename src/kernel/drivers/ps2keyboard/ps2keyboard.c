#include "ps2keyboard.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <task/process.h>

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

void ps2keyboard_irq_handler();

int ps2keyboard_setup()
{
    kprintf("Initializing PS2 Keyboard driver\r\n");
    outb(0x64, 0xAE); // Enable first PS/2 port

    idt_SetHandler(0x21, ps2keyboard_irq_handler);

    // TODO: Self-test

    return 0;
}

static bool ps2keyboard_capslock = false;
static bool ps2keyboard_shift = false;

static char shift_char(char c)
{
    switch(c)
    {
        case '1': return '!';
        case '2': return '@';
        case '3': return '#';
        case '4': return '$';
        case '5': return '%';
        case '6': return '^';
        case '7': return '&';
        case '8': return '*';
        case '9': return '(';
        case '0': return ')';
        case '-': return '_';
        case '=': return '+';
        case '[': return '{';
        case ']': return '}';
        case ';': return ':';
        case '\'': return '"';
        case '`': return '~';
        case '\\': return '|';
        case ',': return '<';
        case '.': return '>';
        case '/': return '?';
        default:  return c;
    }
}

[[maybe_unused]] static char ps2keyboard_scancode_to_char(uint8_t scancode)
{
    // TODO: Handle other scan sets
    size_t size_of_set = sizeof(ps2keyboard_scan_set_1) / sizeof(uint8_t);

    if (scancode > size_of_set)
    {
        return 0;
    }

    char c = ps2keyboard_scan_set_1[scancode];

    if (c >= 'A' && c <= 'Z' && !ps2keyboard_capslock)
        c += 32;

    else if (ps2keyboard_shift) {
        c = shift_char(c);
    }

    return c;
}

void ps2keyboard_irq_handler()
{
    pic_EOI(0);

    uint8_t scancode = inb(0x60);
    inb(0x60); // Ignore next byte

    if (scancode == 0xAA || scancode == 0xB6)  // Left Shift release (0xAA) or Right Shift release (0xB6)
    {
        ps2keyboard_shift = false;
        return;
    }

    if (scancode & 0x80) //release
    {
        return;
    }

    if (scancode == 0x2A || scancode == 0x36)  // Left Shift (0x2A) or Right Shift (0x36)
    {
        ps2keyboard_shift = true;
        return;
    }

    if (scancode == 0x3A) { //Capslock

        ps2keyboard_capslock = !ps2keyboard_capslock;
        return;
    }

    uint8_t c = ps2keyboard_scancode_to_char(scancode);
    if (c != 0)
    {
        process_pushkey(process_current(), c);
    }
}

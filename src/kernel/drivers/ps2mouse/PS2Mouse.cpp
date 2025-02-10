//
// Created by Pawel Reich on 1/26/25.
//

#include "PS2Mouse.hpp"

#include <fs/file.h>

extern "C" {
#include <drivers/pic/pic.h>
#include "kernel.h"
#include <common/io.h>
#include <idt/idt.h>
}

void ps2mouse_irq_handler(int int_no, interrupt_frame* frame) {
    (void)(int_no);
    (void)(frame);
    PS2Mouse::instance()->handle_cycle();
}

void PS2Mouse::handle_cycle() {
    uint8_t status = inb(MOUSE_STATUS);
    while (status & MOUSE_BBIT) {
    int8_t mouse_in = inb(MOUSE_PORT);
    if (status & MOUSE_F_BIT) {
    switch (mouse_cycle) {
        case 0:
            mouse_byte[0] = mouse_in;
        if (!(mouse_in & MOUSE_V_BIT)) return;
        ++mouse_cycle;
        break;
        case 1:
            mouse_byte[1] = mouse_in;
        ++mouse_cycle;
        break;
        case 2:
            mouse_byte[2] = mouse_in;

        if (mouse_byte[0] & 0x80 || mouse_byte[0] & 0x40) {
            break;
        }
        uint8_t mouse_buttons = mouse_byte[0];
        int32_t mouse_x = static_cast<int8_t>(mouse_byte[1]);
        int32_t mouse_y = static_cast<int8_t>(mouse_byte[2]);

        if ((mouse_byte[0] >> 5 & 0x1) == 1)
            mouse_x = mouse_x | (mouse_byte[0] >> 5 & 0x1);
        if ((mouse_byte[0] >> 4 & 0x1) == 1)
            mouse_y = mouse_y | (mouse_byte[0] >> 4 & 0x1);

        mouse_y = -mouse_y;

        mouse_cycle = 0;
        mouse_packet packet = {.x = mouse_x, .y = mouse_y, .buttons = mouse_buttons};
        pipe->write((char*) &packet, sizeof(mouse_packet));

        break;
            }
        }
        status = inb(MOUSE_STATUS);
    }
    pic_EOI(MOUSE_IRQ);
}
PS2Mouse::PS2Mouse() {
    kprintf("Initializing PS2 Mouse\n");
    outb(MOUSE_STATUS, 0xA7); // Disable mouse

    // Flush output buffer
    while (inb(MOUSE_STATUS) & MOUSE_BBIT) {
        inb(MOUSE_PORT);
    }

    // Enable auxiliary device (mouse)
    outb(MOUSE_STATUS, 0xA8);

    // Configure controller
    outb(MOUSE_STATUS, 0x20); // Read configuration byte
    uint8_t status = inb(MOUSE_PORT) | 0x02; // Enable mouse IRQ
    outb(MOUSE_STATUS, 0x60); // Write configuration byte
    outb(MOUSE_PORT, status);

    // Reset mouse
    write(0xFF); // Reset command
    uint8_t ack = read();
    if (ack != 0xFA) {
        kernel_panic("Mouse reset failed");
    }
    uint8_t self_test = read();
    if (self_test != 0xAA) {
        kernel_panic("Mouse self-test failed");
    }
    read();
    // Enable default values
    write(0xF6);
    if (read() != 0xFA) {
        kernel_panic("Mouse enable failed");
    }


    write(0xE8);
    write(0x03);

    write(0xF4);
    // Enable data reporting
    if (read() != 0xFA) {
        kernel_panic("Mouse enable failed");
    }

    // Set up IRQ handler
    idt_SetHandler(0x20 + MOUSE_IRQ, ps2mouse_irq_handler);

    pipe = new PipeFS(sizeof(mouse_packet) * pipe_size);
    mount("0:/mouse", pipe->get_struct(), pipe);
}

inline void PS2Mouse::wait(uint8_t type) const {
    uint32_t timeout = TIMEOUT;
    if(type == 0) {
        while(timeout--) {
            if((inb(MOUSE_STATUS) & MOUSE_BBIT) == 1) {
                return;
            }
        }
        kprintf("a_type=0 timeout");
    }
    else {
        while(timeout--) {
            if((inb(MOUSE_STATUS) & MOUSE_ABIT) == 0) {
                return;
            }
        }
        kprintf("a_type=1 timeout");
    }
}

inline void PS2Mouse::write(uint8_t byte) const {
    wait(1);

    outb(MOUSE_STATUS, MOUSE_WRITE);
    wait(1);

    outb(MOUSE_PORT, byte);
}

uint8_t PS2Mouse::read() const {
    wait(0);
    return inb(MOUSE_PORT);
}

PS2Mouse *PS2Mouse::instance() {
    static PS2Mouse instance;
    return &instance;
}

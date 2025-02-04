//
// Created by Pawel Reich on 2/3/25.
//

#include "compositor.hpp"

#include <drivers/graphics/vbe/vbe_graphics.hpp>


extern "C" {
#include <kernel.h>
#include <memory/memory.h>
#include <task/process.h>
}

Compositor::Compositor() {
    auto graphics = reinterpret_cast<VBEGraphics*>(VBEGraphics::the());

    mouse_x = graphics->get_width()/2;
    mouse_y = graphics->get_height()/2;
    memset(mouse_buf, 0, sizeof(mouse_buf));
}

Compositor* Compositor::instance() {
    static Compositor instance;
    return &instance;
}

void Compositor::push_mouse(int32_t x, int32_t y, uint8_t buttons) {
    if (mouse_buf_idx > mouse_buf_size) {
        kernel_panic("Mouse buf idx overflow");
    }
    mouse_buf[mouse_buf_idx++] = {.x = x, .y = y, .buttons = buttons};
}

void Compositor::draw() {

    struct process** processes = process_get_list();

    auto graphics = reinterpret_cast<VBEGraphics*>(VBEGraphics::the());

    for (int idx = 0; idx < MAX_PROCESSES; idx++) {
        auto* process = processes[idx];
        if (process == 0)
            continue;

        memcpy(graphics->get_framebuffer(), process->framebuffer, graphics->get_framebuffer_size());
    }

    //Should be spin-locked
    __asm__ __volatile__("cli");
    for (uint32_t idx = 0; idx < mouse_buf_idx; idx++) {
        mouse_buf_idx = 0;
        auto packet = mouse_buf[idx];
        mouse_x = mouse_x + packet.x;
        mouse_y = mouse_y + packet.y;
    }
    __asm__ __volatile__("sti");
    for (int idx = -1; idx < 2; idx++) {
        for (int idx1 = -1; idx1< 2; idx1++) {
            graphics->draw_pixel(mouse_x+idx, mouse_y+idx1, 0xFFFFFF);
        }
    }
}

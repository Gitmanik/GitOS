//
// Created by Pawel Reich on 1/23/25.
//

#include "vbe_graphics.hpp"

#include <kernel.h>

#include "drivers/graphics/graphics.hpp"

#include <stdint.h>
#include <fs/memory/MemoryFS.hpp>

extern "C" {
#include <fs/file.h>
#include <memory/memory.h>
}

Graphics* VBEGraphics::the() {
    static VBEGraphics instance;
    return &instance;
}

// Draw a pixel at (x, y) with a given color
void VBEGraphics::draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x >= WIDTH || y >= HEIGHT)
        return;

    uint32_t offset = (y * PITCH) + (x * (BPP / 8));

    if (BPP == 32) {
        FRAMEBUFFER[offset + 0] = color & 0xFF;          // Blue
        FRAMEBUFFER[offset + 1] = (color >> 8) & 0xFF;   // Green
        FRAMEBUFFER[offset + 2] = (color >> 16) & 0xFF;  // Red
    } else if (BPP == 24) {
        FRAMEBUFFER[offset + 0] = color & 0xFF;          // Blue
        FRAMEBUFFER[offset + 1] = (color >> 8) & 0xFF;   // Green
        FRAMEBUFFER[offset + 2] = (color >> 16) & 0xFF;  // Red
        FRAMEBUFFER[offset + 3] = (color >> 24) & 0xFF;  // Alpha
    }
}

// Draw a single character at (x, y)
void VBEGraphics::draw_char(uint32_t x, uint32_t y, char c, uint32_t color) {
    for (int row = 0; row < 8; row++) {
        uint8_t row_bits = font[(int)c - 0x20][row];
        for (int col = 0; col < 8; col++) {
            if (row_bits & (1 << (7 - col))) {
                for (unsigned int xS = 0; xS < FONT_SCALE; xS++) {
                    for (unsigned int yS = 0; yS < FONT_SCALE; yS++) {
                        draw_pixel(x + FONT_SCALE*col+xS, y + FONT_SCALE*row+yS, color);
                    }
                }
            }
        }
    }
}

// Draw a string of characters starting at (x, y)
void VBEGraphics::draw_string(uint32_t x, uint32_t y, const char *str, uint32_t color) {
    while (*str) {
        draw_char(x, y, *str++, color);
        x += 8*FONT_SCALE; // Move to the next character position
    }
}

uint32_t VBEGraphics::get_cursor_y() {
    return current_y;
}
uint32_t VBEGraphics::get_cursor_x() {
    return current_x;
}
Graphics::TEXT_MODE_COLOR VBEGraphics::get_current_color() {
    return current_color;
}

void VBEGraphics::print_char(char c) {
    print_char_color(c, current_color);
}

void VBEGraphics::print_char_color(char c, TEXT_MODE_COLOR color) {
    if (c == '\r') {
        current_x = 0;
    }
    else if (c == '\n') {
        current_y++;
        current_x = 0;
    }

    if (current_x >= WIDTH / (8*FONT_SCALE)) {
        current_x = 0;
        current_y++;
    }
    if (current_y >= HEIGHT / (8*FONT_SCALE)) {
        current_y = HEIGHT / (8*FONT_SCALE) - 1;
        current_x = 0;
        scroll_screen(1);
    }

    if (c == '\n' || c == '\r')
        return;
    draw_char(current_x*8*FONT_SCALE, current_y*8*FONT_SCALE, c, get_rrggbb_color(color));
    current_x++;

}

void VBEGraphics::print_string_color(const char *str, TEXT_MODE_COLOR color) {
    while (*str) {
        print_char_color(*str++, color);
    }
}

void VBEGraphics::print_string(const char *str) {
    print_string_color(str, current_color);
}

void VBEGraphics::set_text_color(TEXT_MODE_COLOR color) {
    current_color = color;
}

void VBEGraphics::scroll_screen(int amount) {
    amount = amount * (8*FONT_SCALE);
    memcpy(FRAMEBUFFER, reinterpret_cast<char *>(FRAMEBUFFER) + PITCH * amount, PITCH * (HEIGHT - amount));
    memset(reinterpret_cast<char*>(FRAMEBUFFER) + PITCH * (HEIGHT - amount), 0, PITCH * amount);

}

void VBEGraphics::set_cursor(uint32_t x, uint32_t y) {
    current_x = x;
    current_y = y;
}

void VBEGraphics::clear_screen() {
    for (uint32_t x = 0; x < WIDTH; x++) {
        for (uint32_t y = 0; y < HEIGHT; y++) {
            draw_pixel(x,y,0);
        }
    }
}

bool VBEGraphics::is_vbe() const {
    return m_setup;
}

size_t VBEGraphics::get_framebuffer_size() const {
    return PITCH * HEIGHT;
}

uint32_t VBEGraphics::get_bpp() const {
    return BPP;
}

uint32_t VBEGraphics::get_height() const {
    return HEIGHT;
}

uint32_t VBEGraphics::get_width() const {
    return WIDTH;
}

uint8_t *VBEGraphics::get_framebuffer() const {
    return FRAMEBUFFER;
}

void VBEGraphics::setup(uint8_t* framebuffer, uint32_t bpp, uint32_t width, uint32_t height)
{
    FRAMEBUFFER = framebuffer;
    BPP = bpp;
    WIDTH = width;
    HEIGHT = height;
    PITCH = WIDTH * BPP / 8;

    kprintf("VBEGraphics set up: %dx%dx%d\r\n", WIDTH, HEIGHT, BPP);

    m_setup = true;
}

void VBEGraphics::mount_fb() {
    m_memoryfs = new MemoryFS(reinterpret_cast<char *>(FRAMEBUFFER), get_framebuffer_size());
    mount("0:/fb", m_memoryfs->get_struct(), m_memoryfs);
}

uint32_t VBEGraphics::get_rrggbb_color(const TEXT_MODE_COLOR color) {
    switch (color) {
        case BLACK:        return 0x000000; // Black
        case BLUE:         return 0x0000AA; // Blue
        case GREEN:        return 0x00AA00; // Green
        case CYAN:         return 0x00AAAA; // Cyan
        case RED:          return 0xAA0000; // Red
        case PURPLE:       return 0xAA00AA; // Purple
        case BROWN:        return 0xAA5500; // Brown
        case GREY:         return 0xAAAAAA; // Grey (light grey)
        case DARK_GREY:    return 0x555555; // Dark Grey
        case LIGHT_BLUE:   return 0x5555FF; // Light Blue
        case LIGHT_GREEN:  return 0x55FF55; // Light Green
        case LIGHT_CYAN:   return 0x55FFFF; // Light Cyan
        case LIGHT_RED:    return 0xFF5555; // Light Red
        case LIGHT_PURPLE: return 0xFF55FF; // Light Purple
        case YELLOW:       return 0xFFFF55; // Yellow
        case WHITE:        return 0xFFFFFF; // White
        default:           return 0x000000; // Default to black for unknown colors
    }
}

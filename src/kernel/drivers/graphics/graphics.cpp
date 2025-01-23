//
// Created by Pawel Reich on 1/23/25.
//

#include "graphics.hpp"

#include <kernel.h>
#include <stdint.h>

Graphics* Graphics::the() {
    static Graphics g;
    return &g;
}

// Draw a pixel at (x, y) with a given color
void Graphics::draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x >= WIDTH || y >= HEIGHT)
        return;

    uint32_t offset = (y * modeInfo->pitch) + (x * (modeInfo->bpp / 8));

    if (modeInfo->bpp == 32) {
        FRAMEBUFFER[offset + 0] = color & 0xFF;          // Blue
        FRAMEBUFFER[offset + 1] = (color >> 8) & 0xFF;   // Green
        FRAMEBUFFER[offset + 2] = (color >> 16) & 0xFF;  // Red
    } else if (modeInfo->bpp == 24) {
        // 24bpp: Write 3 bytes (RGB format, no alpha)
        FRAMEBUFFER[offset + 0] = color & 0xFF;          // Blue
        FRAMEBUFFER[offset + 1] = (color >> 8) & 0xFF;   // Green
        FRAMEBUFFER[offset + 2] = (color >> 16) & 0xFF;  // Red
        FRAMEBUFFER[offset + 3] = (color >> 24) & 0xFF;  // Alpha
    }
}

// Draw a single character at (x, y)
void Graphics::draw_char(uint32_t x, uint32_t y, char c, uint32_t color) {
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
void Graphics::draw_string(uint32_t x, uint32_t y, const char *str, uint32_t color) {
    while (*str) {
        draw_char(x, y, *str++, color);
        x += 8*FONT_SCALE; // Move to the next character position
    }
}

void Graphics::print_char(char c, uint32_t color) {
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
        current_y = 0;
        current_x = 0;
    }

    if (c == '\n' || c == '\r')
        return;
    draw_char(current_x*8*FONT_SCALE, current_y*8*FONT_SCALE, c, color);
    current_x++;

}

void Graphics::print_string(const char *str, uint32_t color) {
    while (*str) {
        print_char(*str++, color);
    }
}
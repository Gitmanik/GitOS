//
// Created by Pawel Reich on 1/25/25.
//

#include "framebuffer.hpp"

#include <stdint.h>
extern "C" {
#include <misc.h>
#include "stdio.h"
#include "string.h"
}

FramebufferGraphics::FramebufferGraphics() {

    FramebufferInfo* fbInfo = (FramebufferInfo*) malloc(sizeof(FramebufferInfo));
    get_framebuffer_info(fbInfo);

    FRAMEBUFFER = fbInfo->buffer;
    BPP = fbInfo->bpp;
    WIDTH = fbInfo->width;
    HEIGHT = fbInfo->height;
    PITCH = WIDTH * BPP / 8;
    free(fbInfo);

    clear_screen();
}

FramebufferGraphics* FramebufferGraphics::the() {
    static FramebufferGraphics instance;
    return &instance;
}

// Draw a pixel at (x, y) with a given color
void FramebufferGraphics::draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x >= WIDTH || y >= HEIGHT)
        return;

    uint32_t offset = y * PITCH + x * BPP/8;

    if (BPP == 32) {
        FRAMEBUFFER[offset + 0] = color & 0xFF;          // Blue
        FRAMEBUFFER[offset + 1] = (color >> 8) & 0xFF;   // Green
        FRAMEBUFFER[offset + 2] = (color >> 16) & 0xFF;  // Red
        FRAMEBUFFER[offset + 3] = (color >> 24) & 0xFF;  // Alpha
    } else if (BPP == 24) {
        // 24bpp: Write 3 bytes (RGB format, no alpha)
        FRAMEBUFFER[offset + 0] = color & 0xFF;          // Blue
        FRAMEBUFFER[offset + 1] = (color >> 8) & 0xFF;   // Green
        FRAMEBUFFER[offset + 2] = (color >> 16) & 0xFF;  // Red
    }
}

uint32_t FramebufferGraphics::get_pixel(uint32_t x, uint32_t y) {
    if (x >= WIDTH || y >= HEIGHT)
        return 0;

    uint32_t offset = y * PITCH + x * BPP/8;

    uint32_t pixel = 0;

    if (BPP == 32) {
        pixel = FRAMEBUFFER[offset + 0];          // Blue
        pixel |= FRAMEBUFFER[offset + 1] << 8;   // Green
        pixel |= FRAMEBUFFER[offset + 2] << 16;  // Red
        pixel |= FRAMEBUFFER[offset + 3] << 24;  // Alpha
    } else if (BPP == 24) {
        pixel = FRAMEBUFFER[offset + 0];          // Blue
        pixel |= FRAMEBUFFER[offset + 1] << 8;   // Green
        pixel |= FRAMEBUFFER[offset + 2] << 16;  // Red
    }
    return pixel;
}

// Draw a single character at (x, y)
void FramebufferGraphics::draw_char(uint32_t x, uint32_t y, char c, uint32_t color) {
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
void FramebufferGraphics::draw_string(uint32_t x, uint32_t y, const char *str, uint32_t color) {
    while (*str) {
        draw_char(x, y, *str++, color);
        x += 8*FONT_SCALE; // Move to the next character position
    }
}

uint32_t FramebufferGraphics::get_cursor_y() {
    return current_y;
}
uint32_t FramebufferGraphics::get_cursor_x() {
    return current_x;
}
uint32_t FramebufferGraphics::get_current_color() {
    return current_color;
}

void FramebufferGraphics::print_char(char c) {
    print_char_color(c, current_color);
}

void FramebufferGraphics::print_char_color(char c, uint32_t color) {
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
    draw_char(current_x*8*FONT_SCALE, current_y*8*FONT_SCALE, c, color);
    current_x++;

}

void FramebufferGraphics::print_string_color(const char *str, uint32_t color) {
    while (*str) {
        print_char_color(*str++, color);
    }
}

void FramebufferGraphics::print_string(const char *str) {
    print_string_color(str, current_color);
}

void FramebufferGraphics::set_text_color(uint32_t color) {
    current_color = color;
}

void FramebufferGraphics::scroll_screen(int amount) {
    amount = amount * (8*FONT_SCALE);
    memcpy(FRAMEBUFFER, reinterpret_cast<char *>(FRAMEBUFFER) + PITCH * amount, PITCH * (HEIGHT - amount));
    memset(reinterpret_cast<char*>(FRAMEBUFFER) + PITCH * (HEIGHT - amount), 0, PITCH * amount);
}

void FramebufferGraphics::set_cursor(uint32_t x, uint32_t y) {
    current_x = x;
    current_y = y;
}

void FramebufferGraphics::clear_screen() {
    for (uint32_t x = 0; x < WIDTH; x++) {
        for (uint32_t y = 0; y < HEIGHT; y++) {
            draw_pixel(x,y,0);
        }
    }
    current_x = 0;
    current_y = 0;
    current_color = 0xAAAAAA;
}
extern "C" {
    void printf(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);

        char internal_buf[1024];
        memset(internal_buf, 0, sizeof(internal_buf));

        vsprintf(internal_buf, fmt, args);

        FramebufferGraphics::the()->print_string(internal_buf);
    }
    void debug_printf(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);

        char internal_buf[1024];
        memset(internal_buf, 0, sizeof(internal_buf));

        vsprintf(internal_buf, fmt, args);

        debug_puts(internal_buf);
    }
    void puts(const char* str) {
        FramebufferGraphics::the()->print_string(str);
    }
    void putc(char c) {
        FramebufferGraphics::the()->print_char(c);
    }
}

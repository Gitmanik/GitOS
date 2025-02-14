//
// Created by Paweł Reich on 2/7/25.
//

#include <format/bmp/BMPFile.hpp>
#include <graphics/framebuffer.hpp>

#include "../../kernel/kernel.h"

extern "C" {
#include "stdio.h"
#include "string.h"
#include "misc.h"
#include "file.h"
}

struct mouse_packet { // TODO: Make this smaller
    int32_t x;
    int32_t y;
    unsigned char buttons;
};

int main(int argc, char** argv) {
    (void)(argc);
    (void)(argv);

    auto* fbg = FramebufferGraphics::the();

    int framebuffer_fd = fopen("0:/fb", "rw");
    int mouse_fd = fopen("0:/mouse", "r");

    uint32_t mouse_x = fbg->get_width()/2;
    uint32_t mouse_y = fbg->get_height()/2;

    // Load and draw background bitmap
    int background_fd = fopen("0:/sys/bg.bmp", "r");
    file_stat background_stat{};
    fstat(background_fd, &background_stat);
    auto background_data = (uint8_t*) malloc(background_stat.filesize);
    fread(background_data, background_stat.filesize, 1, background_fd);
    auto background = new BMPFile(background_data, background_stat.filesize);

    for (uint32_t y = 0; y < background->get_height(); y++) {
        for (uint32_t x = 0; x < background->get_width(); x++) {
            fbg->draw_pixel(x,y,background->get_pixel(x,y));
        }
    }

    fbg->print_string_color("GitOS Shell", 0xffff0000);

    // Load cursor bitmap
    int cursor_fd = fopen("0:/sys/cursor.bmp", "r");
    file_stat cursor_stat{};
    fstat(cursor_fd, &cursor_stat);
    uint8_t* cursor_data = (uint8_t*) malloc(cursor_stat.filesize);
    fread(cursor_data, cursor_stat.filesize, 1, cursor_fd);
    auto cursor = new BMPFile(cursor_data, cursor_stat.filesize);

    auto* under_cursor_buffer = new uint32_t[cursor->get_width() * cursor->get_height() * sizeof(uint32_t)];

    for (uint32_t y = 0; y < cursor->get_height(); y++) {
        for (uint32_t x = 0; x < cursor->get_width(); x++) {
            under_cursor_buffer[y*cursor->get_height() + x] = fbg->get_pixel(mouse_x + x, mouse_y + y);
            auto col = cursor->get_pixel(x,y);
            if ((col & 0xFF000000) == 0)
                continue;
            fbg->draw_pixel(mouse_x + x, mouse_y + y, col);
        }
    }

    fwrite(fbg->get_buffer(), fbg->get_buffer_size(), 1, framebuffer_fd);

    auto* data = new mouse_packet;
    while (true) {
        file_stat stat{};
        while (stat.filesize == 0)
            fstat(mouse_fd, &stat);

        fread(data, sizeof(mouse_packet), 1, mouse_fd);

        for (uint32_t y = 0; y < cursor->get_height(); y++) {
            for (uint32_t x = 0; x < cursor->get_width(); x++) {
                fbg->draw_pixel(mouse_x + x, mouse_y + y, under_cursor_buffer[y*cursor->get_height() + x]);
            }
            int off = fbg->get_offset(mouse_x, mouse_y + y);
            fseek(framebuffer_fd, off, SEEK_SET);
            fwrite(fbg->get_buffer() + off, fbg->get_bpp() / 8 * cursor->get_width(), 1, framebuffer_fd);
        }

        int new_mouse_x = mouse_x + data->x;
        int new_mouse_y = mouse_y + data->y;

        if (new_mouse_x < 1)
            new_mouse_x = 1;
        if (new_mouse_y < 1)
            new_mouse_y = 1;

        if (new_mouse_x >= (int) fbg->get_width())
            new_mouse_x = fbg->get_width();

        if (new_mouse_y >= (int) fbg->get_height())
            new_mouse_y = fbg->get_height();


        for (uint32_t y = 0; y < cursor->get_height(); y++) {
            for (uint32_t x = 0; x < cursor->get_width(); x++) {
                under_cursor_buffer[y*cursor->get_height() + x] = fbg->get_pixel(new_mouse_x + x, new_mouse_y + y);
                auto col = cursor->get_pixel(x,y);
                if ((col & 0xFF000000) == 0) {
                    continue;
                }
                fbg->draw_pixel(new_mouse_x + x, new_mouse_y + y, col);
            }
            int off = fbg->get_offset(new_mouse_x, new_mouse_y + y);
            fseek(framebuffer_fd, off, SEEK_SET);
            fwrite(fbg->get_buffer() + off, cursor->get_width() * fbg->get_bpp()/8, 1, framebuffer_fd);
        }

        mouse_x = new_mouse_x;
        mouse_y = new_mouse_y;
    }

    return 0;
}
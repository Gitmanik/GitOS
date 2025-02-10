//
// Created by Paweł Reich on 2/7/25.
//

#include <graphics/framebuffer.hpp>

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

    int framebuffer_fd = fopen("0:/fb", "rw");
    int mouse_fd = fopen("0:/mouse", "r");

    auto* fbg = FramebufferGraphics::the();

    uint32_t framebuffer_sz = fbg->get_width() * fbg->get_bpp() / 8 * fbg->get_height();

    fbg->print_string_color("GitOS Shell", 0x00ff00);

    fwrite(fbg->get_buffer(), framebuffer_sz, 1, framebuffer_fd);

    uint32_t mouse_x = fbg->get_width()/2;
    uint32_t mouse_y = fbg->get_height()/2;

    uint32_t under_cursor_buffer[3*3]{};

    mouse_packet* data = (mouse_packet*) malloc(sizeof(mouse_packet));
    while (1) {
        file_stat stat{};
        while (stat.filesize == 0)
            fstat(mouse_fd, &stat);

        fread(data, sizeof(mouse_packet), 1, mouse_fd);

        for (int y = 0; y < 3; y++) {
            for (int x = 0; x < 3; x++) {
                fbg->draw_pixel(mouse_x + x, mouse_y + y, under_cursor_buffer[y*3 + x]);
            }
            int off = fbg->get_offset(mouse_x, mouse_y + y);
            fseek(framebuffer_fd, off, SEEK_SET);
            fwrite(fbg->get_buffer() + off, fbg->get_bpp() / 8 * 3, 1, framebuffer_fd);
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


        for (int y = 0; y < 3; y++) {
            for (int x = 0; x < 3; x++) {
                under_cursor_buffer[y*3 + x] = fbg->get_pixel(new_mouse_x + x, new_mouse_y + y);
                fbg->draw_pixel(new_mouse_x + x, new_mouse_y + y, 0xFFFFFF);
            }
            int off = fbg->get_offset(new_mouse_x, new_mouse_y + y);
            fseek(framebuffer_fd, off, SEEK_SET);
            fwrite(fbg->get_buffer() + off, 3 * fbg->get_bpp()/8, 1, framebuffer_fd);
        }

        mouse_x = new_mouse_x;
        mouse_y = new_mouse_y;
    }

    return 0;
}
//
// Created by Paweł Reich on 2/7/25.
//

#include <format/bmp/BMPFile.hpp>
#include <graphics/framebuffer.hpp>
#include <window/window.h>

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

struct window
{
    uint32_t pid;
    uint32_t width;
    uint32_t height;


    uint32_t x;
    uint32_t y;

    uint8_t* fd = nullptr;
    char* name;
    bool dirty;

    uint32_t old_x;
    uint32_t old_y;
    bool moving = false;
    bool finished_moving = false;
};

int main(int argc, char** argv) {
    (void)(argc);
    (void)(argv);

    debug_printf("Starting GitOS Graphical Shell..\n");

    open_ipc("WindowServer", 1024*512, 1);
    auto ipc_fd = fopen("0:/ipc/WindowServer", "rw");

    execprocess("0:/wintest.elf");

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

    auto* windows = new window[32]; // TODO: This should be a list
    auto* tmp_window_message_preamble = new window_message_preamble;
    auto* tmp_window_message_set_size = new window_message_set_size;
    auto* tmp_char = new char;

    uint32_t title_bar_sz = 24;

    while (true) {
        file_stat stat{};
        file_stat ipc_stat{};

        fstat(ipc_fd, &ipc_stat);
        fstat(mouse_fd, &stat);
        if (ipc_stat.filesize > 0)
        {
            fread(tmp_window_message_preamble, sizeof(*tmp_window_message_preamble), 1, ipc_fd);

            auto window = &windows[tmp_window_message_preamble->pid]; // TODO: This should be a list traversal

            if (tmp_window_message_preamble->type == WINDOW_MESSAGE::REGISTER)
            {
                window->name = new char[255];
                int ctr = 0;
                do
                {
                    fread(tmp_char, 1, 1, ipc_fd);
                    window->name[ctr] = *tmp_char;
                    ctr++;
                } while (*tmp_char != 0);
            }
            else if (tmp_window_message_preamble->type == WINDOW_MESSAGE::SET_SIZE)
            {
                fread(tmp_window_message_set_size, sizeof(*tmp_window_message_set_size), 1, ipc_fd);
                window->height = tmp_window_message_set_size->height;
                window->width = tmp_window_message_set_size->width;
                if (window->fd != nullptr)
                    delete window->fd;

                window->fd = new uint8_t[window->width * window->height * 3];
            }
            else if (tmp_window_message_preamble->type == WINDOW_MESSAGE::FRAMEBUFFER_DATA)
            {
                fread(window->fd, window->width * window->height * 3, 1, ipc_fd);
                window->dirty = true;
            }
            else if (tmp_window_message_preamble->type == WINDOW_MESSAGE::CLOSE)
            {
                if (window->fd != nullptr)
                {
                    delete window->fd;
                    window->fd = nullptr;
                }
                else
                {
                    debug_printf("PID %d tried to close nonexistent window!\n");
                    while (1);
                }
            }
        }

        for (int i = 0; i < 32; i++) // TODO: This should be a list traversal
        {
            auto window = &windows[i]; // TODO: This should be a list traversal

            if (window->fd == nullptr)
                continue;
            if (!window->dirty)
                continue;
            if (window->moving)
                continue;

            for (uint32_t y = 0; y < title_bar_sz; y++) {
                int off = fbg->get_offset( window->x, window->y + y);
                for (uint32_t x = 0; x < window->width; x++)
                {
                    fbg->draw_pixel(window->x + x, window->y + y, 0xffffffff);
                }
                fbg->draw_string(window->x, window->y, window->name, 0x00ff00ff);
                fseek(framebuffer_fd, off, SEEK_SET);
                fwrite(fbg->get_buffer() + off, fbg->get_bpp() / 8 * window->width, 1, framebuffer_fd);
            }

            for (uint32_t y = 0; y < window->height; y++) {
                int off = fbg->get_offset(window->x, window->y + title_bar_sz + y);
                memcpy(fbg->get_buffer() + off, window->fd + y*window->width * 3, window->width * 3);
                fseek(framebuffer_fd, off, SEEK_SET);
                fwrite(fbg->get_buffer() + off, fbg->get_bpp() / 8 * window->width, 1, framebuffer_fd);
            }

            window->dirty = false;
        }

        if (stat.filesize == 0)
        {
            continue;
        }

        fread(data, sizeof(mouse_packet), 1, mouse_fd);



        for (int i = 0; i < 32; i++) // TODO: This should be a list traversal
        {
            auto window = &windows[i]; // TODO: This should be a list traversal

            if (window->fd == nullptr)
                continue;

            if ((data->buttons & 1) == 1 && mouse_x > window->x && mouse_x - window->x < window->width && mouse_y > window->y && mouse_y - window->y < title_bar_sz)
            {
                if (!window->moving)
                {
                    window->old_x = window->x;
                    window->old_y = window->y;
                    window->moving = true;
                }

                window->x += data->x;
                window->y += data->y;
            } else if ((data->buttons & 1) == 0 && window->moving)
            {
                window->moving = false;
                window->finished_moving = true;
            }

            if (window->finished_moving)
            {
                for (uint32_t y = 0; y < window->height + title_bar_sz; y++) {
                    for (uint32_t x = 0; x < window->width; x++) {
                        fbg->draw_pixel(window->old_x + x,window->old_y + y,background->get_pixel(window->old_x + x,window->old_y + y));
                    }
                    int off = fbg->get_offset(window->old_x, window->old_y + y);
                    fseek(framebuffer_fd, off, SEEK_SET);
                    fwrite(fbg->get_buffer() + off, fbg->get_bpp() / 8 * window->width, 1, framebuffer_fd);
                }
                for (uint32_t y = 0; y < cursor->get_height(); y++)
                    for (uint32_t x = 0; x < cursor->get_width(); x++)
                        under_cursor_buffer[y*cursor->get_height() + x] = fbg->get_pixel(window->old_x + x, window->old_y + y);

                window->finished_moving = false;
            }
        }

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
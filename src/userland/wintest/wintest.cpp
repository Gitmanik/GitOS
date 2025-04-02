//
// Created by Paweł Reich on 4/2/25.
//

#include <stdint-gcc.h>
#include <graphics/framebuffer.hpp>
#include <window/window.h>

extern "C" {
#include <file.h>
#include <string.h>
#include <misc.h>
#include <stdio.h>
}

int main(int argc, char *argv[]) {
    (void)(argc);
    (void)(argv);

    auto ws_fd = fopen("0:/ipc/WindowServer", "rw");

    auto* fb = new FramebufferGraphics(200, 200, 24);

    fb->print_string_color("h", 0xffff0000);

    auto* preamble = new window_message_preamble;
    preamble->pid = getpid();

    preamble->type = WINDOW_MESSAGE::REGISTER;
    fwrite(preamble, sizeof(window_message_preamble), 1, ws_fd);
    char* name = new char[255];
    memset(name, 0, 255);
    strcpy(name, "Window Test\0");
    fwrite(name, sizeof(char), strlen(name)+1, ws_fd);

    preamble->type = WINDOW_MESSAGE::SET_SIZE;
    fwrite(preamble, sizeof(window_message_preamble), 1, ws_fd);
    auto* wrd = new window_message_set_size;
    wrd->width = fb->get_width();
    wrd->height = fb->get_height();
    fwrite(wrd, sizeof(window_message_set_size), 1, ws_fd);

    preamble->type = WINDOW_MESSAGE::FRAMEBUFFER_DATA;

    uint8_t* full_data = (uint8_t*) malloc(sizeof(window_message_preamble) + fb->get_buffer_size());
    memcpy(full_data, preamble, sizeof(window_message_preamble));

    while (true)
    {
        for (int i = 0; i < 100000000; i++)
            __asm__ volatile("nop");
        fb->print_string_color("i", 0xffff0000);

        memcpy(full_data+sizeof(window_message_preamble), fb->get_buffer(), fb->get_buffer_size());
        fwrite(full_data, sizeof(window_message_preamble) + fb->get_buffer_size(), 1, ws_fd);
    }
    return 0;
}
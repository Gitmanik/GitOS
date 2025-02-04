//
// Created by Paweł Reich on 2/3/25.
//

#pragma once
#include <stdint-gcc.h>

class Compositor {

    public:
        static Compositor* instance();

        void push_mouse(int32_t x, int32_t y, uint8_t buttons);

        void draw();

    private:
        Compositor();

        uint32_t mouse_buf_idx = 0;

        struct mouse_packet {
            int32_t x;
            int32_t y;
            uint8_t buttons;
        };

        constexpr static uint32_t mouse_buf_size = 128;
        mouse_packet mouse_buf[mouse_buf_size];
        uint32_t mouse_x = 0, mouse_y = 0;
};

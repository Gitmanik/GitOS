//
// Created by Pawel Reich on 1/25/25.
//

#pragma once
#include <stdint.h>

struct FramebufferInfo {
    uint8_t* buffer;
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
};

extern "C" {
    void get_framebuffer_info(struct FramebufferInfo* fbInfo);
}


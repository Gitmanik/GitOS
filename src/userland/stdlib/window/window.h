//
// Created by Pawel Reich on 4/2/25.
//

#pragma once

#include <stdint-gcc.h>

enum WINDOW_MESSAGE
{
    REGISTER,
    SET_SIZE,
    FRAMEBUFFER_DATA,
    CLOSE
};

struct window_message_preamble
{
    uint32_t pid;
    WINDOW_MESSAGE type;
} __attribute__((packed));

struct window_message_set_size
{
    uint32_t height;
    uint32_t width;
} __attribute__((packed));

struct window_message_framebuffer_data
{
    uint8_t* data;
} __attribute__((packed));
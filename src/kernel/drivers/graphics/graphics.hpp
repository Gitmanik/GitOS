//
// Created by Pawel Reich on 1/23/25.
//

#pragma once

#include <stdint.h>

class Graphics {
  public:
    virtual ~Graphics() = default;

    enum TEXT_MODE_COLOR
    {
        BLACK,
        BLUE,
        GREEN,
        CYAN,
        RED,
        PURPLE,
        BROWN,
        GREY,
        DARK_GREY,
        LIGHT_BLUE,
        LIGHT_GREEN,
        LIGHT_CYAN,
        LIGHT_RED,
        LIGHT_PURPLE,
        YELLOW,
        WHITE
    };

    static Graphics* the();

    virtual void print_char(char c) = 0;
    virtual void print_char_color(char c, TEXT_MODE_COLOR color)  = 0;

    virtual void print_string(const char* str)  = 0;
    virtual void print_string_color(const char* str, TEXT_MODE_COLOR color)  = 0;

    virtual void clear_screen()  = 0;

    virtual void set_text_color(TEXT_MODE_COLOR color)  = 0;
    virtual void set_cursor(uint32_t x, uint32_t y)  = 0;

    virtual uint32_t get_cursor_x()  = 0;
    virtual uint32_t get_cursor_y()  = 0;
    virtual TEXT_MODE_COLOR get_current_color()  = 0;
};

#pragma once

enum TEXT_MODE_COLORS
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
    LIGHT_RED,
    LIGHT_PURPLE,
    YELLOW,
    WHITE
};

#define VGA_TEXT_WIDTH  80
#define VGA_TEXT_HEIGHT 25

void print_char(char c, enum TEXT_MODE_COLORS fg);
void print_string(const char* str);
void clear_screen();
void set_fg(enum TEXT_MODE_COLORS col);
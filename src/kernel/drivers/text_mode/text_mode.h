#pragma once

enum TEXT_MODE_COLOURS
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

void print_char(char c, enum TEXT_MODE_COLOURS fg);
void print_string(char* str);
#include "text_mode.h"
#include <stdint.h>
#include <stddef.h>

int current_x = 0, current_y = 0;
enum TEXT_MODE_COLORS current_fg = GREY;

uint16_t* video_mem =  (uint16_t*) 0xb8000;

void clear_screen()
{
    current_x = current_y = 0;
    for (int y = 0; y < VGA_TEXT_HEIGHT; y++)
        for (int x = 0; x < VGA_TEXT_WIDTH; x++)
        print_char(' ', BLACK);

    current_x = current_y = 0;
}

uint16_t make_char(char c, enum TEXT_MODE_COLORS col)
{
    return (col << 8) | c;
}

void print_char(char c, enum TEXT_MODE_COLORS fg)
{
    if (c == '\n')
    {
        current_y++;
        current_x = 0;
        return;
    }
    if (current_x == VGA_TEXT_WIDTH)
    {
        current_x = 0;
        current_y++;
    }

    if (current_y == VGA_TEXT_HEIGHT)
    {
        current_y = 0;
        current_x = 0;
    }

    video_mem[current_y * VGA_TEXT_WIDTH + current_x] = make_char(c, fg);
    current_x++;
}

size_t strlen(const char* str)
{
    size_t len = 0;
    while(str[len] != 0)
    {
        len++;
    }
    return len;
}

void print_string(const char* str)
{
    size_t len = strlen(str);

    for (int idx = 0; idx < len; idx++)
    {
        print_char(str[idx], current_fg);
    }
}

void set_fg(enum TEXT_MODE_COLORS new_fg)
{
    current_fg = new_fg;
}
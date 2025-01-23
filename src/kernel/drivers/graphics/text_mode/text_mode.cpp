#include "drivers/graphics/graphics.hpp"
#include "text_mode.hpp"
#include <stdint.h>
#include <stddef.h>

extern "C" {
#include "common/string.h"
#include "memory/memory.h"
}

Graphics* TextModeGraphics::the()
{
    // Singleton pattern: static local instance
    static TextModeGraphics instance;
    return &instance;
}

/**
 * @brief Creates memory entry
 * 
 * @param c Character
 * @param color Color
 * @return uint16_t Entry to put in video memory
 */
uint16_t TextModeGraphics::make_char(const char c, const TEXT_MODE_COLOR color)
{
    return (color << 8) | c;
}

/**
 * @brief Clears screen
 * 
 */
void TextModeGraphics::clear_screen()
{
    memset(video_mem, 0, (TEXT_MODE_HEIGHT * TEXT_MODE_WIDTH) * 2);
    current_x = current_y = 0;
}


/**
 * @brief Prints char to screen
 * 
 * @param c Character to print
 */

void TextModeGraphics::print_char(const char c)
{
    print_char_color(c, current_fg);
}

/**
 * @brief Prints char to screen with specified color
 * 
 * @param c Character to print
 * @param color Color
 */
void TextModeGraphics::print_char_color(const char c, const TEXT_MODE_COLOR color)
{
    if (c == '\n')
    {
        current_y++;
        current_x = 0;
    }
    if (c == '\r')
    {
        current_x = 0;
    }
    
    if (current_x == TEXT_MODE_WIDTH)
    {
        current_x = 0;
        current_y++;
    }

    if (current_y == TEXT_MODE_HEIGHT)
    {
        current_y = TEXT_MODE_HEIGHT - 1;
        current_x = 0;
        scroll_screen(1);
    }
    if (c == '\n' || c == '\r')
        return;

    video_mem[current_y * TEXT_MODE_WIDTH + current_x] = make_char(c, color);
    current_x++;
}

/**
 * @brief Prints string to screen
 * 
 * @param str String to print
 */
void TextModeGraphics::print_string(const char* str)
{
    print_string_color(str, current_fg);
}

/**
 * @brief Prints string to screen with specified color
 * 
 * @param str String to print
 * @param color Color
 */
void TextModeGraphics::print_string_color(const char* str, const TEXT_MODE_COLOR color)
{
    for (size_t idx = 0; idx < strlen(str); idx++)
    {
        print_char_color(str[idx], color);
    }
}

/**
 * @brief Sets new framebuffer color to print
 * 
 * @param color New color to be set
 */
void TextModeGraphics::set_text_color(const TEXT_MODE_COLOR color)
{
    current_fg = color;
}

/**
 * @brief Sets framebuffer cursor to specified values
 * 
 * @param x X to be set
 * @param y Y to be set
 */
void TextModeGraphics::set_cursor(const uint32_t x, const uint32_t y)
{
    current_x = x;
    current_y = y;
}

/**
 * @brief Scrolls framebuffer
 * 
 * @param amount Amount of lines to scroll
 */
void TextModeGraphics::scroll_screen(int amount)
{
    memcpy(video_mem, reinterpret_cast<char *>(video_mem) + 2 * TEXT_MODE_WIDTH * amount, 2 * TEXT_MODE_WIDTH * (TEXT_MODE_HEIGHT - amount));
    memset(reinterpret_cast<char*>(video_mem) + 2 * TEXT_MODE_WIDTH * (TEXT_MODE_HEIGHT - amount), 0, 2 * TEXT_MODE_WIDTH * amount);
}

/**
 * @return TEXT_MODE_COLORS Current color
 */
Graphics::TEXT_MODE_COLOR TextModeGraphics::get_current_color()
{
    return current_fg;
}

/**
 * @return int X
 */
uint32_t TextModeGraphics::get_cursor_x()
{
    return current_x;
}

/**
 * @return int Y
 */
uint32_t TextModeGraphics::get_cursor_y()
{
    return current_y;
}
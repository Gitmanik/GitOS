#include "text_mode.h"
#include <stdint.h>
#include <stddef.h>
#include "../../common/string.h"
#include "../../memory/memory.h"

static int current_x = 0, current_y = 0;
static enum TEXT_MODE_COLORS current_fg = GREY;

static uint16_t* video_mem =  (uint16_t*) 0xb8000;

/**
 * @brief Creates memory entry
 * 
 * @param c Character
 * @param col Color
 * @return uint16_t Entry to put in video memory
 */
static uint16_t make_char(char c, enum TEXT_MODE_COLORS col)
{
    return (col << 8) | c;
}

/**
 * @brief Clears screen
 * 
 */
void tm_ClearScreen()
{
    memset(video_mem, 0, (TEXT_MODE_HEIGHT * TEXT_MODE_WIDTH) * 2);
    current_x = current_y = 0;
}


/**
 * @brief Prints char to screen
 * 
 * @param c Character to print
 */

void tm_PrintChar(char c)
{
    tm_PrintCharColor(c, current_fg);
}

/**
 * @brief Prints char to screen with specified color
 * 
 * @param c Character to print
 * @param fg Color
 */
void tm_PrintCharColor(char c, enum TEXT_MODE_COLORS fg)
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
        tm_ScrollScreen(1);
    }
    if (c == '\n' || c == '\r')
        return;

    video_mem[current_y * TEXT_MODE_WIDTH + current_x] = make_char(c, fg);
    current_x++;
}

/**
 * @brief Prints string to screen
 * 
 * @param str String to print
 */
void tm_PrintString(const char* str)
{
    tm_PrintStringColor(str, current_fg);
}

/**
 * @brief Prints string to screen with specified color
 * 
 * @param str String to print
 * @param col Color
 */
void tm_PrintStringColor(const char* str, enum TEXT_MODE_COLORS col)
{
    for (size_t idx = 0; idx < strlen(str); idx++)
    {
        tm_PrintCharColor(str[idx], col);
    }
}

/**
 * @brief Sets new framebuffer color to print
 * 
 * @param new_fg New color to be set
 */
void tm_SetColor(enum TEXT_MODE_COLORS new_fg)
{
    current_fg = new_fg;
}

/**
 * @brief Sets framebuffer cursor to specified values
 * 
 * @param new_x X to be set
 * @param new_y Y to be set
 */
void tm_SetCursor(int new_x, int new_y)
{
    current_x = new_x;
    current_y = new_y;
}

/**
 * @brief Scrolls framebuffer
 * 
 * @param amount Amount of lines to scroll
 */
void tm_ScrollScreen(int amount)
{
    memcpy((void*)video_mem, (void*) video_mem + 2 * TEXT_MODE_WIDTH * amount, 2 * TEXT_MODE_WIDTH * (TEXT_MODE_HEIGHT - amount));
    memset((void*) video_mem + 2 * TEXT_MODE_WIDTH * (TEXT_MODE_HEIGHT - amount), 0, 2 * TEXT_MODE_WIDTH * amount);
}

/**
 * @return TEXT_MODE_COLORS Current color
 */
int tm_GetColor()
{
    return current_fg;
}

/**
 * @return int X
 */
int tm_GetX()
{
    return current_x;
}

/**
 * @return int Y
 */
int tm_GetY()
{
    return current_y;
}
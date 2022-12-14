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
    LIGHT_CYAN,
    LIGHT_RED,
    LIGHT_PURPLE,
    YELLOW,
    WHITE
};

#define TEXT_MODE_WIDTH  80
#define TEXT_MODE_HEIGHT 25

void tm_PrintChar(char c);
void tm_PrintCharColor(char c, enum TEXT_MODE_COLORS fg);
void tm_PrintString(const char* str);
void tm_PrintStringColor(const char* str, enum TEXT_MODE_COLORS fg);

void tm_ClearScreen();
void tm_ScrollScreen(int amount);

void tm_SetCursor(int new_x, int new_y);

void tm_SetColor(enum TEXT_MODE_COLORS col);
int tm_GetColor();

int tm_GetX();
int tm_GetY();
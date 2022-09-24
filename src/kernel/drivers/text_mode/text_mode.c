#include "text_mode.h"


int current_x = 0, current_y = 0;

char* video_mem =  (char*) 0xb8000;
void print_char(char c, enum TEXT_MODE_COLOURS fg)
{
    video_mem[(current_y * 80 + current_x)*2] = c;
    video_mem[((current_y * 80 + current_x)*2)+1] = fg;
}

void print_string(char* str)
{
    
}
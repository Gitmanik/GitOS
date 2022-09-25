#include "kernel.h"
#include "drivers/text_mode/text_mode.h"

void kernel_main()
{
    tm_ClearScreen();
    tm_SetColor(LIGHT_GREEN);
    tm_PrintString("GitOS - operating system as exercise. Pawel Reich 2022\nLoading..");

    tm_SetCursor(10,10);
    tm_PrintChar('c', LIGHT_RED);
    kernel_halt();
}

void kernel_halt()
{
    for (;;);
}
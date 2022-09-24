#include "kernel.h"
#include "drivers/text_mode/text_mode.h"

void kernel_main()
{
    clear_screen();
    set_fg(LIGHT_GREEN);
    print_string("GitOS - operating system as exercise. Pawel Reich 2022\nLoading..");
    kernel_halt();
}

void kernel_halt()
{
    for (;;);
}
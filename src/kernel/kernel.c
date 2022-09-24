#include "kernel.h"
#include "drivers/text_mode/text_mode.h"

void kernel_main()
{
    print_char('g', LIGHT_BLUE);
    kernel_halt();
}

void kernel_halt()
{
    for (;;);
}
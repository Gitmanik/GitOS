#include "kernel.h"
#include "drivers/text_mode/text_mode.h"
#include "idt/idt.h"

void divide_by_zero()
{
    tm_PrintString("Division by zero!");
    for (;;);
}

extern void div0test();

void kernel_main()
{
    tm_ClearScreen();
    tm_SetColor(GREY);
    tm_PrintString("GitOS - operating system as exercise. Pawel Reich 2022\n");

    tm_PrintString("Initializing IDT..");

    idt_Init();
    idt_SetDescriptor(0, divide_by_zero);
    idt_Load();

    tm_SetColor(LIGHT_GREEN);
    tm_PrintString(" OK\n");
    tm_SetColor(GREY);

    int test = 1/0;

    kernel_halt();
}

void kernel_halt()
{
    for (;;);
}
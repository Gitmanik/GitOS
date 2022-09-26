#include "kernel.h"
#include "drivers/text_mode/text_mode.h"
#include "idt/idt.h"
#include "drivers/serial/serial.h"

void divide_by_zero()
{
    tm_PrintString("Division by zero!");
    for (;;);
}

void kernel_main()
{
    ser_Init(COM1, 1);
    tm_ClearScreen();
    kernel_message("GitOS - operating system as exercise. Pawel Reich 2022\r\n", GREY);
    kernel_message("Initializing IDT..", GREY);

    idt_Init();
    idt_SetDescriptor(0, divide_by_zero);
    idt_Load();

    kernel_message("OK\r\n",LIGHT_GREEN);

    while (1)
    {
        while (!ser_IsAvailable(COM1));
        char c = ser_ReadChar(COM1);
        tm_PrintChar(c, LIGHT_PURPLE);
        if (c == '\r')
            tm_PrintChar('\n', LIGHT_PURPLE);
    }

    kernel_halt();
}

void kernel_halt()
{
    for (;;);
}

void kernel_message(char* message, enum TEXT_MODE_COLORS col)
{
    enum TEXT_MODE_COLORS x = tm_GetColor();

    tm_SetColor(col);
    tm_PrintString(message);
    tm_SetColor(x);

    ser_PrintString(COM1, message);
}
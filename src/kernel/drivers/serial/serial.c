#include "serial.h"
#include <stddef.h>
#include "../../common/io.h"
#include "../../common/string.h"
// https://wiki.osdev.org/Serial_Ports

int ser_Init(uint16_t port, uint16_t divisor)
{
    outb(port + 1, 0); // Disable interrupts
    outb(port + 3, 0b10000000); //Enable DLAB
    outb(port + 0, divisor & 0x00ff); //Divisor high byte
    outb(port + 1, (divisor >> 8) & 0x00ff);//  low byte
    outb(port + 3, 0b00000010); // Line Control Register
    outb(port + 2, 0b11000111); //TODO : ocb
    outb(port + 4, 0x0b);
    outb(port + 4, 0x1e);
    outb(port + 0, 0xAE); // Send test byte

    if (inb(port + 0) != 0xAE)
    {
        return 1;
    }

    outb(port + 4, 0x0f);
    return 0;
}

void ser_PrintChar(uint16_t port, char c)
{
    outb(port, c);
}
void ser_PrintString(uint16_t port, const char* str)
{
    size_t sz = strlen(str);
    for (size_t idx = 0; idx < sz; idx++)
        ser_PrintChar(port, str[idx]);
}

int ser_IsAvailable(uint16_t port)
{
    return inb(port + 5) & 0x1;
}

char ser_ReadChar(uint16_t port)
{
    return inb(port);
}
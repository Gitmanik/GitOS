#include "serial.h"
#include <stddef.h>
#include "../../common/io.h"
#include "../../common/string.h"
#include "../../common/status.h"
// https://wiki.osdev.org/Serial_Ports

/**
 * @brief Initializes specified Serial port
 * 
 * @param port Serial port address pointer
 * @param divisor 0-65535 divisor for 115200 base speed
 * @return int Status
 */
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
        return -EIO;
    }

    outb(port + 4, 0x0f);
    return ALL_OK;
}

/**
 * @brief Prints single character to Serial port
 * 
 * @param port Serial port pointer
 * @param c Character to write
 */
void ser_PrintChar(uint16_t port, char c)
{
    outb(port, c);
}

/**
 * @brief Prints string to specified Serial port
 * 
 * @param port Serial port pointer
 * @param str String to write
 */
void ser_PrintString(uint16_t port, const char* str)
{
    size_t sz = strlen(str);
    for (size_t idx = 0; idx < sz; idx++)
        ser_PrintChar(port, str[idx]);
}

/**
 * @brief Checks if port has unread value
 * 
 * @param port Serial port pointer
 * @return int 1 if can read
 */
int ser_IsAvailable(uint16_t port)
{
    return inb(port + 5) & 0x1;
}

/**
 * @brief Reads single character from Serial port
 * 
 * @param port Serial port pointer
 * @return char Read character
 */
char ser_ReadChar(uint16_t port)
{
    return inb(port);
}
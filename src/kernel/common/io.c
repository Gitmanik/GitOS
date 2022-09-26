#include "io.h"
#include <stdint.h>

uint8_t inb(uint16_t port)
{
    char val;
    asm volatile ("inb %1, %0" : "=a" (val) : "d" (port));
    return val;
}

uint16_t inw(uint16_t port)
{
    short val;
    asm volatile ("inw %1, %0" : "=a" (val) : "d" (port));
    return val;
}

void outb(uint16_t port, uint8_t val)
{
    asm volatile ("outb %0, %1" : : "a" (val), "d" (port));
}
void outw(uint16_t port, uint16_t val)
{
    asm volatile ("outw %0, %1" : : "a" (val), "d" (port));
}
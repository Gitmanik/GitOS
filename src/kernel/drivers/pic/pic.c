#include "pic.h"
#include <stdint.h>
#include "../../common/io.h"

#define ICW1_ICW4	0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */
 
#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

#define END_OF_INTERRUPT 0x20

/**
 * @brief Remaps master PIC and slave PIC to specified interupts numbers
 * 
 * @param offset1 New starting interrupt number for master PIC (IRQ 0-7)
 * @param offset2 New starting interrupt number for slave PIC (IRQ 8-15)
 */
void pic_Remap(uint8_t offset1, uint8_t offset2)
{
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

    outb(PIC1_DATA, offset1);
    outb(PIC2_DATA, offset2);

    outb(PIC1_DATA, 4);
    outb(PIC2_DATA, 2);

    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    // // Mask all IRQs except IRQ1 (keyboard) and IRQ12 (mouse)
    outb(PIC1_DATA, 0b11111000); // Unmask only IRQ0 (timer), IRQ1 and IRQ2 (for IRQ12 to work)
    outb(PIC2_DATA, 0b11101111); // Unmask only IRQ12
}

/**
 * @brief Sets PIT Channel 0 frequency
 * @param hz Desired PIT Hz
 */
void pic_SetHz(uint16_t hz)
{
    uint16_t divisor = 1193182 / hz;
    outb(0x43, 0x36);  // Channel 0, lobyte/hibyte, square wave
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}

/**
 * @brief Sends End Of Interrupt to PIC
 * 
 * @param irq Number of IRQ to EOI
 */
void pic_EOI(unsigned char irq)
{
    if (irq >= 8) // Reset slave
        outb(PIC2_COMMAND, END_OF_INTERRUPT);

    outb(PIC1_COMMAND, END_OF_INTERRUPT);
}
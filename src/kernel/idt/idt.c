#include "idt.h"
#include <stdint.h>
#include "memory/memory.h"
#include "drivers/pic/pic.h"

struct idt_desc idt_descriptors[512];
struct idtr_desc idtr_descriptor;

/**
 * @brief Loads Interrupt Descriptor Table
 * 
 */
void idt_Load()
{
    asm volatile ("lidt %0" : : "m" (idtr_descriptor));
}

/**
 * @brief Fills entry in IDT for specified Interrupt number
 * Use before idt_Load()
 * @param int_no Interrupt number
 * @param address Pointer to interrupt handler
 */
void idt_SetDescriptor(int int_no, void* address)
{
    struct idt_desc* desc = &idt_descriptors[int_no];

    desc->offset_low = (uint32_t) address & 0x0000ffff;
    desc->offset_high = ((uint32_t) address & 0xffff0000) >> 16;
    desc->selector = 0x08; //Code
    desc->zero = 0;
    desc->type_attr = 0b11101110; // P 1b, DPL 2b, S 1b, Type 4b
}

/**
 * @brief Initializes memory for IDT struct
 * Sets default for all interrupts
 */
void idt_Init()
{
    memset(idt_descriptors, 0, sizeof(idt_descriptors));

    idtr_descriptor.limit = sizeof(idt_descriptors)-1;
    idtr_descriptor.base = (uint32_t) idt_descriptors;

    for (int i = 0; i < 512; i++)
        idt_SetDescriptor(i, ignore_int);
}

/**
 * @brief Default handler for all interrupts
 * 
 */
void ignore_int_handler()
{
    pic_EOI(0);
}


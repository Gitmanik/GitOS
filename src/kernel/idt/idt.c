#include "idt.h"
#include <stdint.h>
#include "../memory/memory.h"

idt_desc idt_descriptors[512];
idtr_desc idtr_descriptor;

void idt_Load()
{
    asm volatile ("lidt %0" : : "m" (idtr_descriptor));
}

void idt_SetDescriptor(int int_no, void* address)
{
    idt_desc* desc = &idt_descriptors[int_no];

    desc->offset_low = (uint32_t) address & 0x0000ffff;
    desc->offset_high = ((uint32_t) address & 0xffff0000) >> 16;
    desc->selector = 0x08; //Code
    desc->zero = 0;
    desc->type_attr = 0b11101110; // P 1b, DPL 2b, S 1b, Type 4b
}

void idt_Init()
{
    memset(idt_descriptors, 0, sizeof(idt_descriptors));

    idtr_descriptor.limit = sizeof(idt_descriptors)-1;
    idtr_descriptor.base = (uint32_t) idt_descriptors;
}
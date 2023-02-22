#include "gdt.h"
#include "kernel.h"

struct gdt_descriptor gdt_descriptor = {0};

void encode_gdt_entry(uint8_t *target, struct gdt_structured src)
{
    if ((src.limit > 65536) && (src.limit & 0xFFF) != 0xFFF)
    {
        kernel_panic("encode_gdt_entry: Invalid limit");
    }

    // Encode limit
    if (src.limit > 65536) // Set granularity bit
    {
        src.limit = src.limit >> 12; // Divide by 2^12 (4096).
        target[6] = 0b11000000;
    }
    else
    {
        target[6] = 0b01000000;
    }
    target[6] |= (src.limit >> 16) & 0x0F;

    target[0] = src.limit & 0xFF;
    target[1] = (src.limit >> 8) & 0xFF;

    // Encode base
    target[2] = src.base & 0xFF;
    target[3] = (src.base >> 8) & 0xFF;
    target[4] = (src.base >> 16) & 0xFF;
    target[7] = (src.base >> 24) & 0xFF;

    // Set type
    target[5] = src.type;
}

void gdt_structured_to_gdt(struct gdt *gdt, struct gdt_structured *structured_gdt, unsigned int total_entries)
{
    for (unsigned int i = 0; i < total_entries; i++)
    {
        encode_gdt_entry((uint8_t *)&gdt[i], structured_gdt[i]);
    }
}

void gdt_load(struct gdt *gdt, unsigned int size)
{
    // kprintf("Loading GDT in %x, sz: %d\n", gdt, size);
    gdt_descriptor.size = size;
    gdt_descriptor.start_address = (uint32_t)gdt;

    asm volatile("lgdt %0" : : "m" (gdt_descriptor));
}

void gdt_read(struct gdt_descriptor *target)
{
    asm volatile ("sgdt %0" : "=m"(*target));
}
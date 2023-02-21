#pragma once

#include <stdint.h>

struct gdt
{
    uint16_t segment;
    uint16_t base_first;
    uint8_t base;
    uint8_t access;
    uint8_t high_flags;
    uint8_t base_24_31_bits;
};

struct gdt_structured
{
    uint32_t base;
    uint32_t limit;
    uint8_t type;
};

struct gdt_descriptor 
{
    uint16_t size;
    uint32_t start_address;
} __attribute__((packed));

void gdt_load(struct gdt *gdt, unsigned int size);
void gdt_read(struct gdt_descriptor **target);
void gdt_structured_to_gdt(struct gdt *gdt, struct gdt_structured *structured_gdt, unsigned int total_entries);

#define TOTAL_GDT_SEGMENTS 3
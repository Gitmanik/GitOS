#pragma once
#include <stdint.h>

typedef struct __attribute__((packed))
{
    uint16_t offset_low; //Offset bits 0-15
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;   
    uint16_t offset_high; //Offset bits 16-31
} idt_desc;

typedef struct __attribute__((packed))
{
    uint16_t limit; // Size of desc table - 1
    uint32_t base; // Address of the desc table
} idtr_desc;

void idt_Init();
void idt_SetDescriptor(int int_no, void* address);
void idt_Load();
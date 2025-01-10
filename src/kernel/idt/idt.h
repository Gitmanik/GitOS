#pragma once
#include <stdint.h>

#define MAX_INTERRUPTS 256

struct idt_desc
{
    uint16_t offset_low; //Offset bits 0-15
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;   
    uint16_t offset_high; //Offset bits 16-31
} __attribute__((packed));

struct idtr_desc
{
    uint16_t limit; // Size of desc table - 1
    uint32_t base; // Address of the desc table
} __attribute__((packed));

struct interrupt_frame
{
    uint32_t error_code;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t reserved;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t esp;
    uint32_t ss;
} __attribute__((packed));

typedef void(*ISR_HANDLER)(int int_no, struct interrupt_frame* frame);

void idt_Init();
void idt_SetDescriptor(int int_no, void* address);
int idt_SetHandler(int int_no, ISR_HANDLER handler);
void idt_Load();

extern const char *idt_InterruptLayoutString[32];

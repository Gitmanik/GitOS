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

typedef void(*ISR_HANDLER)(struct interrupt_frame* frame);

void idt_Init();
void idt_SetDescriptor(int int_no, void* address);
int idt_SetHandler(int int_no, ISR_HANDLER handler);
void idt_Load();


/*
0x000	0	Divide by 0
0x004	1	Single step (Debugger)
0x008	2	Non Maskable Interrupt (NMI) Pin
0x00C	3	Breakpoint (Debugger)
0x010	4	Overflow
0x014	5	Bounds check
0x018	6	Undefined Operation Code (OPCode) instruction
0x01C	7	No coprocessor
0x020	8	Double Fault
0x024	9	Coprocessor Segment Overrun
0x028	10	Invalid Task State Segment (TSS)
0x02C	11	Segment Not Present
0x030	12	Stack Segment Overrun
0x034	13	General Protection Fault (GPF)
0x038	14	Page Fault
0x03C	15	Unassigned
0x040	16	Coprocessor error
0x044	17	Alignment Check (486+ Only)
0x048	18	Machine Check (Pentium/586+ Only)
0x05C	19-31	Reserved exceptions
0x068 - 0x3FF	32-255	Interrupts free for software use
*/
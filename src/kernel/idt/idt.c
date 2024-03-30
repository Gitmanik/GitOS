#include "idt.h"
#include <stdint.h>
#include "common/status.h"
#include "memory/memory.h"
#include "drivers/pic/pic.h"
#include "task/task.h"
#include "kernel.h"

struct idt_desc idt_descriptors[MAX_INTERRUPTS];
struct idtr_desc idtr_descriptor;

extern void* interrupt_pointer_table[MAX_INTERRUPTS];
ISR_HANDLER interrupt_handlers[MAX_INTERRUPTS];

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
 * @brief Sets handler for given interrupt number
 * 
 * @param int_no Interrupt number
 * @param handler Handler function
 * @return int Error code
 */
int idt_SetHandler(int int_no, ISR_HANDLER handler)
{
    if (int_no < 0 || int_no >= MAX_INTERRUPTS-1)
    {
        return -EINVARG;
    }

    interrupt_handlers[int_no] = handler;    

    return ALL_OK;
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

    for (int i = 0; i < MAX_INTERRUPTS; i++)
        idt_SetDescriptor(i, interrupt_pointer_table[i]);
}

/**
 * @brief Handles interrupt (called by ISR)
 * 
 * @param int_no Interrupt number
 * @param frame Interrupt frame
 */
void idt_Handler(int int_no, struct interrupt_frame* frame)
{
    kernel_page();
    if (interrupt_handlers[int_no] != 0)
    {
        task_current_save_state(frame);
        interrupt_handlers[int_no](frame);
    }
    task_page();
}
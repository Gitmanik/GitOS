#include "kernel.h"

#include <drivers/ps2mouse/PS2Mouse.hpp>

#include "drivers/graphics/graphics.hpp"
#include "drivers/graphics/vbe/vbe_graphics.hpp"
#include "drivers/graphics/text_mode/text_mode.hpp"
#include "syscall/syscall.hpp"
#include "compositor/compositor.hpp"

extern "C"
{
#include <stdint.h>
#include <stdarg.h>
#include "idt/idt.h"
#include "drivers/serial/serial.h"
#include "drivers/pic/pic.h"
#include "memory/bios_memory_map.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "common/io.h"
#include "common/status.h"
#include "common/string.h"
#include "memory/paging/paging.h"
#include "drivers/disk/disk.h"
#include "drivers/disk/disk_streamer.h"
#include "fs/pathparser.h"
#include "fs/file.h"
#include "fs/fat16/fat16.h"
#include "gdt/gdt.h"
#include "task/tss.h"
#include "task/task.h"
#include "task/process.h"
#include "drivers/ps2keyboard/ps2keyboard.h"
}

extern "C" int atexit(void (*)())
{
    return 0;
}

void *operator new(size_t size)
{
    return kzalloc(size);
}

void *operator new[](size_t size)
{
    return kzalloc(size);
}

void operator delete(void *p)
{
    kfree(p);
}

void operator delete[](void *p)
{
    kfree(p);
}
static struct paging_chunk *kernel_paging_chunk;

static Graphics* get_graphics() {
    if (static_cast<VBEGraphics*>(VBEGraphics::the())->is_vbe())
        return VBEGraphics::the();
    else
        return TextModeGraphics::the();
}


struct tss tss;
struct gdt gdt_real[TOTAL_GDT_SEGMENTS];
struct gdt_structured gdt_structured[TOTAL_GDT_SEGMENTS] = {
    {.base = 0x00, .limit = 0x00, .type = 0x00},       // NULL
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0x9A}, // KERNEL CODE
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0x92}, // KERNEL DATA
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0xF8}, // USER CODE
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0xF2}, // USER DATA
    {.base = (uint32_t)&tss, .limit = sizeof(tss), .type = 0xE9}, // TASK SWITCH SEGMENT
};

void print_interrupt_frame(struct interrupt_frame* frame)
{
    kprintf("Interrupt frame:\nedi: %d (0x%x)\nesi: %d (0x%x)\nebp: %d (0x%x)\nebx: %d (0x%x)\nedx: %d (0x%x)\necx: %d (0x%x)\neax: %d (0x%x)\nip: 0x%x\nflags: 0b%b\nesp: 0x%x\ncs: 0x%x\nss: 0x%x\nerror: %d\n",
    frame->edi, frame->edi,
    frame->esi, frame->esi,
    frame->ebp, frame->ebp,
    frame->ebx, frame->ebx,
    frame->edx, frame->edx,
    frame->ecx, frame->ecx,
    frame->eax, frame->eax,
    frame->ip, frame->flags, frame->esp, frame->cs, frame->ss, frame->error_code);
}

/**
 * @brief Temporary PIC Timer interrupt handler
 * 
 * @param frame Interrupt frame
 */
void timer_interrupt(int int_no, struct interrupt_frame* frame)
{
    (void)(int_no);
    (void)(frame);
    Compositor::instance()->draw();
    pic_EOI(0);
}

void kernel_exception(int int_no, struct interrupt_frame* frame) {

    uint32_t cr0, cr2;

    asm volatile (
        "mov %%cr0, %0"
        : "=r" (cr0) // Output
    );

    asm volatile (
        "mov %%cr2, %0"
        : "=r" (cr2) // Output
    );

    char internal_buf[1024];
    memset(internal_buf, 0, sizeof(internal_buf));
    get_graphics()->set_text_color(Graphics::LIGHT_RED);

    if ((frame->cs & 0x3) == 3)
        ksprintf(internal_buf, "\n\nProcess %s crashed! Exception thrown by CPU: %s\n", process_current()->filename, idt_InterruptLayoutString[int_no]);
    else
        ksprintf(internal_buf, "\n\nKernel panic! Exception thrown by CPU: %s\n", idt_InterruptLayoutString[int_no]);

    ser_PrintString(COM1, internal_buf);
    get_graphics()->print_string(internal_buf);

    print_interrupt_frame(frame);

    get_graphics()->set_text_color(Graphics::GREY);
    memset(internal_buf, 0, sizeof(internal_buf));

    ksprintf(internal_buf,"cr0: 0b%b\ncr2: %d (0x%x)\n\n", cr0, cr2, cr2);
    get_graphics()->print_string(internal_buf);
    ser_PrintString(COM1, internal_buf);

    memset(internal_buf, 0, sizeof(internal_buf));
    ksprintf(internal_buf, "Stack trace:\n");
    uint32_t *ebp = (uint32_t *)frame->ebp;

    while (ebp != 0) {
        uint32_t returnAddress = ebp[1];

        memset(internal_buf, 0, sizeof(internal_buf));
        ksprintf(internal_buf, "0x%x\n", returnAddress);

        get_graphics()->print_string(internal_buf);
        ser_PrintString(COM1, internal_buf);


        // Move ebp up to the caller's frame
        ebp = (uint32_t *)ebp[0];  // dereference saved EBP
    }

    if ((frame->cs & 0x3) == 3) {
        process_terminate(task_current()->process);
        task_switch(process_current()->task);
        task_return(&task_current()->registers);
    }
    else {
        ser_PrintString(COM1, "GitOS halted.");
        get_graphics()->print_string("GitOS halted.");
        while (1);
    }
}

void kernel_page()
{
    kernel_registers();
    paging_switch(kernel_paging_chunk);
}

/**
 * @brief Kernel C entry point
 *
 */
void kernel_main()
{
    int res = 0;

    get_graphics()->clear_screen();
    get_graphics()->set_text_color(Graphics::GREY);
    res = ser_Init(COM1, 1);
    if (res < 0)
    {
        kernel_panic("Could not initialize Serial port!");
    }

    kprintf("GitOS - operating system as exercise.\nCreated by Pawel Reich. BUILD %s %s\n", __DATE__, __TIME__);

    // Initialize GDT
    kprintf("Initializing GDT..");
    memset(gdt_real, 0, sizeof(gdt_real));
    gdt_structured_to_gdt(gdt_real, gdt_structured, TOTAL_GDT_SEGMENTS);
    gdt_load(gdt_real, sizeof(gdt_real)-1);

    struct gdt_descriptor gdt_content;
    gdt_read(&gdt_content);
    kprintf(" GDTR pointing at %x ", gdt_content.start_address);
    kprintf("OK\r\n");
    //

    // Initialize IDT
    kprintf("Initializing IDT..");
    idt_Init();

    for (int i = 0; i < 0x20; i++)
        idt_SetHandler(i, kernel_exception);

    idt_SetHandler(0x20, timer_interrupt);
    idt_SetDescriptor(0x80, (void*) syscall_wrapper);
    idt_Load();
    kprintf("OK\r\n");
    //

    // Initialize TSS
    kprintf("Initializing TSS..");
    memset(&tss,0,sizeof(tss));
    tss.esp0 = 0x600000;
    tss.ss0 = KERNEL_DATA_SELECTOR;
    tss_load(sizeof(struct gdt) * 5); //TSS Segment is 6th GDT Segment

    // ksprintf test
    kprintf("kprintf test: %p %x %i %s %c %ld %%\r\n", bios_memory_map, 0x41424344, -1234, "gitmanik.dev", 'X', __LONG_MAX__);
    kdebug("kdebug test: %p %x %i %s %c %ld %%", bios_memory_map, 0x41424344, -1234, "gitmanik.dev", 'X', __LONG_MAX__);
    //

    // Finding biggest usable memory chunk to use as heap
    memory_map_entry heap_entry;
    int idx = 0;

    kprintf("Usable memory map:\r\n");
    while (bios_memory_map[idx].length_in_bytes > 0)
    {
        if (bios_memory_map[idx].type != 1)
            goto skip;

        kprintf("0x%p -> 0x%p, Size: %ldKB\r\n",
                (long)bios_memory_map[idx].base_address,
                (long)bios_memory_map[idx].base_address + (long)bios_memory_map[idx].length_in_bytes,
                (long)bios_memory_map[idx].length_in_bytes / 1024);

        if (bios_memory_map[idx].length_in_bytes > heap_entry.length_in_bytes)
            heap_entry = bios_memory_map[idx];

    skip:
        idx++;
    }

    // Kernel is located at 0x100000
    if (heap_entry.length_in_bytes < 0x100000)
    {
        kernel_panic("Not enough memory to place kernel heap!");
    }
    heap_entry.base_address += 0x100000;
    heap_entry.length_in_bytes -= 0x100000;

    kprintf("Heap address: 0x%p, Size: %ldKB\r\n", (long)heap_entry.base_address, (long)heap_entry.length_in_bytes / 1024);

    res = kheap_init((void *)(uint32_t)heap_entry.base_address, heap_entry.length_in_bytes);
    if (res < 0)
    {
        kernel_panic("Failed to create heap!");
    }
    //

    kprintf("Enabling paging..");
    kernel_paging_chunk = paging_new_directory(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    paging_switch(kernel_paging_chunk);
    paging_enable();
    kprintf("OK\r\n");

    // Paging test
    kprintf("Setting up paging..");
    char* ptr_real = new char[4096];
    char* ptr_virt = (char *)0x1000;
    res = paging_map_to(kernel_paging_chunk, ptr_virt, ptr_real, paging_align_address(ptr_real + sizeof(ptr_real)), PAGING_ACCESS_FROM_ALL | PAGING_IS_PRESENT | PAGING_IS_WRITEABLE);
    if (res < 0)
    {
        kernel_panic("Could not map virtual address!");
    }

    ((char*) ptr_virt)[0] = 'O';
    ((char*) ptr_virt)[1] = 'K';

    kprintf("OK\r\n");
    kprintf("Paging self-test: 0x%x:'%s' 0x%x:'%s'..", (uint32_t)ptr_real, ptr_real, (uint32_t)ptr_virt, ptr_virt);
    if (memcmp(ptr_real, ptr_virt, 2) != 0)
    {
        kernel_panic("\r\nPaging self-test unsuccessful!");
    }
    kprintf("OK\r\n");
    delete ptr_real;
    //

    // Remap PIC
    kprintf("Remapping PIC..");

    pic_Remap(0x20, 0x28);

    kprintf(" OK\r\n");
    //

    // Initializing PS2 drivers
    ps2keyboard_setup();
    PS2Mouse::instance();

    // Initializing FS
    fs_init();
    fs_insert_filesystem(fat16_init_filesystem());
    disk_search_and_init();
    //

    // Initialize syscalls
    kprintf("Initializing syscalls..");
    syscall_init();
    kprintf("OK\r\n");
    //

    kprintf("Loading zofia.elf..");
    struct process* process = new struct process;
    res = process_load_switch("0:/zofia.elf", process);
    process->argc = 2;
    process->argv[0] = (char*) "zofia.elf";
    process->argv[1] = (char*) "hello:)";

    if (res != 0)
    {
        kernel_panic("Failed to load process");
    }
    kprintf("OK\r\n");

    kprintf("Running task..\r\n");

    task_run_first_ever_task();

    kernel_panic("Reached the end of kernel_main!");
}

/**
 * @brief (Temporary) Prints kernel_message and halts the kernel.
 *
 * @param fmt Reason of the panic
 * @param ... Arguments
 */
void kernel_panic(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char internal_buf[1024];
    memset(internal_buf, 0, sizeof(internal_buf));
    kvsprintf(internal_buf, fmt, args);

    ser_PrintString(COM1, "Kernel panic!\r\n");
    ser_PrintString(COM1, internal_buf);

    get_graphics()->print_string_color("Kernel panic!\r\n", Graphics::LIGHT_RED);
    get_graphics()->print_string_color(internal_buf, Graphics::LIGHT_RED);
    kernel_halt();
}

void kernel_halt()
{
    for (;;)
        ;
}

/**
 * @brief Prints to kernel debug channels. Max length of processed message is 1024 characters.
 *
 * @param fmt Message to format and print.
 * @param ... Arguments
 */
void kprintf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char internal_buf[1024];
    memset(internal_buf, 0, sizeof(internal_buf));

    kvsprintf(internal_buf, fmt, args);
    ser_PrintString(COM1, internal_buf);
}

extern "C" void __cxa_pure_virtual()
{
    // If we ever hit this, we've attempted to call a pure virtual method.
    // In a kernel, you might hang, panic, or debug log.
    // For example:
    for (;;) {
        kernel_panic("__cxa_pure_virtual");
    }
}
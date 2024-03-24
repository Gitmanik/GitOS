BITS 32

extern kernel_main
global _start
_start:
.jump_to_kernel:
    jmp 0x08:kernel_main

; Sets all registers to point to kernel segment
global kernel_registers
kernel_registers:
    mov ax, 0x10 ; Offset for kernel code in GDT
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov fs, ax
    ret

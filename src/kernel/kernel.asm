BITS 32

extern kernel_main
global _start
section .kernelstart

extern stack_top

_start:
    mov esp, stack_top
    push ebx
    push eax
    call kernel_main

; Sets all registers to point to kernel segment
global kernel_registers
kernel_registers:
    mov ax, 0x10 ; Offset for kernel code in GDT
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov fs, ax
    ret

BITS 32

extern kernel_main

global start
global _start
_start:
start:
.setup_pm_registers:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x00200000
    mov esp, ebp
.enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al
.jump_to_kernel:
    jmp 0x08:kernel_main
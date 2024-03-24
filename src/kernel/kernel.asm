BITS 32

extern kernel_main
global _start
_start:
.jump_to_kernel:
    jmp 0x08:kernel_main

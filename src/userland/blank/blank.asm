[BITS 32]

global _start
_start:

label:
    mov eax, 0
    int 0x80
    jmp label
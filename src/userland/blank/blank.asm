[BITS 32]

global _start
_start:
    push str
    mov eax, 2 ; Print
    int 0x80
    add esp, 4

    jmp $

section .data
str:
    db 'Hello from usermode!', 0

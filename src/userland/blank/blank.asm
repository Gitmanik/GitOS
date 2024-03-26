[BITS 32]

global _start
_start:

    mov eax, 0 ; Blank
    int 0x80

    push 20
    push 30
    mov eax, 1 ; Sum
    int 0x80
    add esp, 8

    push str
    mov eax, 2 ; Print
    int 0x80
    add esp, 4

    jmp $

section .data
str:
    db 'Hello from usermode!', 0

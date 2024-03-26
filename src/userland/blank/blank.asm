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

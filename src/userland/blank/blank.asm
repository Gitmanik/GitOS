[BITS 32]

global _start
_start:
    push str
    mov eax, 2
    int 0x80
    add esp,4

    loop:

    call waitforkey
    push eax
    mov eax, 4
    int 0x80
    add esp, 4
    jmp loop

waitforkey:
    mov eax, 3
    int 0x80
    cmp eax, 0x00
    je waitforkey
    ret

section .data
str:
    db 'Hello from usermode! Press a key to show it on screen!', 0

[BITS 32]

extern getc, putc, puts
global main
main:
    push str
    call puts

loop:
    call waitforkey
    push eax
    call putc
    jmp loop

waitforkey:
    call getc
    cmp eax, 0x00
    je waitforkey
    ret

section .data
str:
    db 'Hello from usermode! Press a key to show it on screen!', 0

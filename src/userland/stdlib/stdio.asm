[BITS 32]
global getc
getc:
    push ebp
    mov ebp, esp

    mov eax, 0
    int 0x80

    mov esp, ebp
    pop ebp
    ret

global debug_putc
debug_putc:
    push ebp
    mov ebp, esp

    push dword [ebp+8]

    mov eax, 1
    int 0x80

    add esp, 4

    mov esp, ebp
    pop ebp
    ret

global debug_puts
debug_puts:
    push ebp
    mov ebp, esp

    push dword [ebp+8]

    mov eax, 2
    int 0x80

    add esp, 4

    mov esp, ebp
    pop ebp
    ret
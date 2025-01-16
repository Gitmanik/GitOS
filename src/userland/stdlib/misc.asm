[BITS 32]

global execprocess
execprocess:
    push ebp
    mov ebp, esp

    push dword [ebp+8]
    mov eax, 3
    int 0x80

    add esp, 4

    mov esp, ebp
    pop ebp
    ret

extern malloc
malloc:
    push ebp
    mov ebp, esp

    push dword [ebp+8]
    mov eax, 4
    int 0x80

    add esp, 4

    mov esp, ebp
    pop ebp
    ret

extern free
free:
    push ebp
    mov ebp, esp

    push dword [ebp+8]
    mov eax, 5
    int 0x80

    add esp, 4

    mov esp, ebp
    pop ebp
    ret


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

extern get_process_arguments
get_process_arguments:
    push ebp
    mov ebp, esp

    push dword [ebp+12]
    push dword [ebp+8]
    mov eax, 6
    int 0x80
    add esp, 8

    mov esp, ebp
    pop ebp
    ret

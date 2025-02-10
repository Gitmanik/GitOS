[BITS 32]

global fopen
fopen:
    push ebp
    mov ebp, esp

    push dword [ebp+12]
    push dword [ebp+8]
    mov eax, 9
    int 0x80

    add esp, 8

    mov esp, ebp
    pop ebp
    ret

global fread
fread:
    push ebp
    mov ebp, esp

    push dword [ebp+20]
    push dword [ebp+16]
    push dword [ebp+12]
    push dword [ebp+8]
    mov eax, 10
    int 0x80

    add esp, 16

    mov esp, ebp
    pop ebp
    ret

global fstat
fstat:
    push ebp
    mov ebp, esp

    push dword [ebp+12]
    push dword [ebp+8]
    mov eax, 11
    int 0x80

    add esp, 8

    mov esp, ebp
    pop ebp
    ret

global fseek
fseek:
    push ebp
    mov ebp, esp

    push dword [ebp+16]
    push dword [ebp+12]
    push dword [ebp+8]
    mov eax, 12
    int 0x80

    add esp, 12

    mov esp, ebp
    pop ebp
    ret

global fclose
fclose:
    push ebp
    mov ebp, esp

    push dword [ebp+8]
    mov eax, 13
    int 0x80

    add esp, 4

    mov esp, ebp
    pop ebp
    ret

global fwrite
fwrite:
    push ebp
    mov ebp, esp

    push dword [ebp+20]
    push dword [ebp+16]
    push dword [ebp+12]
    push dword [ebp+8]
    mov eax, 14
    int 0x80

    add esp, 16

    mov esp, ebp
    pop ebp
    ret
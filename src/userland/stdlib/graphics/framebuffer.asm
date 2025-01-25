[BITS 32]

extern get_framebuffer_info
get_framebuffer_info:
    push ebp
    mov ebp, esp

    push dword [ebp+8]
    mov eax, 8
    int 0x80
    add esp, 4

    mov esp, ebp
    pop ebp
    ret

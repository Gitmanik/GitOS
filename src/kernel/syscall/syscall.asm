extern syscall_handler
global syscall_wrapper
syscall_wrapper:
    ; Already pushed:
    ; uint32_t ip
    ; uint32_t cs
    ; uint32_t flags
    ; uint32_t sp
    ; uint32_t ss
    pushad
    
    push esp ; struct interrupt_frame*
    push eax ; Command code
    call syscall_handler
    mov dword[syscall_handler_result], eax
    add esp, 8

    popad
    mov eax, [syscall_handler_result]
    iretd

section .data
syscall_handler_result:
    dd 0

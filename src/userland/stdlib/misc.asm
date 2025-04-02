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

extern exit
exit:
    push ebp
    mov ebp, esp

    push dword [ebp+8]
    mov eax, 7
    int 0x80
    add esp, 4

    mov esp, ebp
    pop ebp
    ret

; void open_ipc(const char* filename, uint32_t packet_size, uint32_t count);
extern open_ipc
open_ipc:
    push ebp
    mov ebp, esp

    push dword [ebp+16]
    push dword [ebp+12]
    push dword [ebp+8]

    mov eax, 15
    int 0x80
    add esp, 12

    mov esp, ebp
    pop ebp
    ret

;int getpid();
extern getpid
getpid:
    push ebp
    mov ebp, esp

    mov eax, 16
    int 0x80

    mov esp, ebp
    pop ebp
    ret
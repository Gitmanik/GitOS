[BITS 32]

section .asm

; struct registers*
global restore_general_purpose_registers
restore_general_purpose_registers:
    push ebp
    mov ebp, esp
    mov ebx, [ebx+8]
    mov edi, [ebx]
    mov esi, [ebx+4]
    mov ebp, [ebx+8]
    mov edx, [ebx+16]
    mov ecx, [ebx+20]
    mov eax, [ebx+24]
    mov ebx, [ebx+12]
    pop esp
    ret

; struct registers*
global task_return
task_return:
    mov ebp, esp

    mov ebx, [ebp+4]

    ;push data segment
    push dword [ebx+44]

    ;push stack pointer
    push dword [ebx+40]

    ;push flags
    pushf
    pop eax
    or eax, 0x200 ;enable interrupts
    push eax

    ;push code segment
    push dword [ebx+32]

    ;push ip
    push dword [ebx+28]

    mov ax, [ebx+44]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push dword [ebx+4]
    call restore_general_purpose_registers
    add esp,4

    iretd

global user_registers
user_registers: ;0x23 is user data segment offset in GDT
    mov ax, 0x23
    mov ds, ax
    mov fs, ax
    mov es, ax
    mov gs, ax
    ret
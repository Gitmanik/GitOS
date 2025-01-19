[BITS 32]

section .asm

; struct registers*
global task_return
task_return:
    mov ebp, esp

    mov ebx, [ebp+4]

    ;push data segment (ss)
    push dword [ebx+44]

    ;push stack pointer (esp)
    push dword [ebx+40]

    ;push flags
    push dword [ebx+36]

    ;push code segment (cs)
    push dword [ebx+32]

    ;push program counter (ip)
    push dword [ebx+28]

    ;setup segment registers
    mov ax, [ebx+44]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov edi, [ebx]
    mov esi, [ebx+4]
    mov ebp, [ebx+8]
    mov edx, [ebx+16]
    mov ecx, [ebx+20]
    mov eax, [ebx+24]

    mov ebx, [ebx+12] ; Last because of storing struct registers* in it

    ;execute in userland
    iretd

global user_registers
user_registers:
    ;0x23 is user data segment offset in GDT
    mov ax, 0x23
    mov ds, ax
    mov fs, ax
    mov es, ax
    mov gs, ax
    ret
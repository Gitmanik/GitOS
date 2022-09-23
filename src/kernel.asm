[BITS 32]

global start

start:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x00200000
    mov esp, ebp
.enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al
    jmp $
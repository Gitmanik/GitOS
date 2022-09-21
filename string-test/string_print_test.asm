ORG 0x7c00
BITS 16

start:
    mov si, message
    call print

    jmp $

print:
    mov bx, 0
.loop:
    lodsb ; loads from si to al, increments si
    cmp al,0 ; checks if null-terminate is loaded from lodsb (string)
    je .done

    call print_char
    jmp .loop
.done:
    ret

print_char:
    mov ah, 0x0e
    int 0x10
    ret

message: db 'gitmania.pl', 0

times 510 - ($ - $$) db 0
db 0x55
db 0xaa


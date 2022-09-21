ORG 0
BITS 16

jmp short setup ; jump over BIOS Parameter Block's space
nop

times 33 db 0 ; make space for BPB
db 0 ; aligning
setup:
    jmp 0x7c0:.setup2 ; set code segment
.setup2:
    cli

    mov ax, 0x7c0 ; data is stored in 0x7c00 (BIOS loads this code to 0x7c00 )
    mov ds, ax 
    mov es, ax

    mov ax, 0x00
    mov ss, ax
    mov sp, 0x7c00 ; set stack below this code

    sti

    ; setting up int0 on IVT (interrupt vector table)
    mov word[ss:0x00], int0_handler ; offset
    mov word[ss:0x02], 0x7c0 ; segment

start:
    mov si, message
    call print_string

    int 0

    jmp $

print_string:
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

int0_handler:
    mov al, 'G'
    call print_char
    iret

message: db 'gitmania.pl', 0

times 510 - ($ - $$) db 0
db 0x55
db 0xaa


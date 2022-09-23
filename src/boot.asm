ORG 0x7c00
BITS 16

jmp short setup ; jump over BIOS Parameter Block's space
nop

times 33 + 1 db 0 ; make space for BPB (+1 for aligning)

setup:
    jmp 0:.setup2 ; set code segment
.setup2:
    cli
    mov ax, 0 ; data is stored in 0x7c00 (BIOS loads this code to 0x7c00 )
    mov ds, ax 
    mov es, ax
    mov ax, 0x00
    mov ss, ax
    mov sp, 0x7c00 ; set stack below this code
    sti
.load_protected_mode:
    cli
    lgdt[gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp 0x08:load32

[BITS 32]
load32:
    mov eax,1 ; LBA
    mov ecx, 100 ; sectors number
    mov edi, 0x0100000 ; load into
    call ata_lba_read
    jmp 0x08:0x0100000


ata_lba_read:
    
    mov ebx,eax ; backup LBA

    shr eax,24 ; send highest 8bits of LBA
    or eax, 0xE0
    mov dx, 0x1f6
    out dx, al

    mov eax,ecx ; send total sectors to read
    mov dx, 0x1f2
    out dx, al
    
    mov eax, ebx 
    mov dx, 0x1f3
    out dx, al

    mov eax,ebx
    mov dx, 0x1f4
    shr eax,8
    out dx, al

    mov eax,ebx
    mov dx, 0x1f5
    shr eax,16
    out dx, al

    mov dx, 0x1f7
    mov al, 0x20
    out dx, al

.next_sector:
    push ecx
.poll:
    mov dx, 0x1f7
    in al, dx
    test al, 8
    jz .poll

    mov ecx, 256
    mov dx, 0x1f0
    rep insw ; loads word from port in DX to EDI, ECX (256) times
    pop ecx
    loop .next_sector
    ret


; Global Descriptor Table
gdt_start:
gdt_null:
    dd 0x0
    dd 0x0

; offset 0x8
gdt_code:
    dw 0xffff     ; Segment limit first 0-15 bits
    dw 0          ; Base first 0-15 bits
    db 0          ; Base 16-23 bits
    db 10011010b  ; Access byte
    db 11001111b  ; High 4 bit flags and the low 4 bit flags
    db 0          ; Base 24-31 bits

; offset 0x10
gdt_data: 
    dw 0xffff     ; Segment limit first 0-15 bits
    dw 0          ; Base first 0-15 bits
    db 0          ; Base 16-23 bits
    db 10010010b  ; Access byte
    db 11001111b  ; High 4 bit flags and the low 4 bit flags
    db 0          ; Base 24-31 bits

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

times 510 - ($ - $$) db 0
db 0x55
db 0xaa
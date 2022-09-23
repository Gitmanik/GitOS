ORG 0x7c00
BITS 16

jmp short setup ; jump over BIOS Parameter Block's space
nop

times 33 + 1 db 0 ; make space for BPB (+1 for alining for easier deassembling later)

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

%include "./src/boot/ata.asm"

[BITS 32]
load32:
    mov eax, 1 ; LBA
    mov ecx, 8 ; sectors number
    mov edi, 0x0100000 ; load into
    call ata_lba_read
    jmp 0x08:0x0100000

[BITS 16]
%include "./src/boot/gdt.asm"

; Aligning to 512 bytes
times 510 - ($ - $$) db 0
db 0x55
db 0xaa
ORG 0x7c00
BITS 16

jmp short setup ; jump over BIOS Parameter Block's space
nop

%include "fat16_bpb.asm"

setup:
    jmp 0:.setup2 ; set code segment
.setup2:
    cli
    mov ax, 0 ; data is stored in 0x7c00 (BIOS loads this code to 0x7c00 )
    mov ds, ax 
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov es, ax

    mov sp, 0x7c00 ; set stack below this code
    call get_memory_map ; Load RAM info into 0x500
    sti
.load_protected_mode:
    cli
    lgdt[gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp 0x08:load32

%include "ata.asm"

[BITS 32]
load32:
    .setup_pm_registers:
    mov ax, 0x10 ; Kernel data segment offset in GDT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x00200000
    mov esp, ebp

    mov eax, 1 ; LBA
    mov ecx, 100 ; sectors number
    mov edi, 0x0100000 ; load into
    call ata_lba_read

    .enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al
    
    jmp 0x08:0x0100000

[BITS 16]
%include "memmap.asm"
%include "gdt.asm"
; Aligning to 512 bytes
times 510 - ($ - $$) db 0
db 0x55
db 0xaa
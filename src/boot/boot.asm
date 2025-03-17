ORG 0x7c00
BITS 16

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
.setup_vbe:
    mov ax, 0x4F02       ; VBE function: Set mode
    mov bx, 0x4118       ; Mode 118h (1024x768x32bpp) + Linear framebuffer
    int 0x10             ; Call BIOS interrupt

    ; Check if the mode was set successfully
    cmp ax, 0x004F       ; AX == 0x004F indicates success
    jne fail

    ; Query VBE mode information
    mov ax, 0x200       ; Load segment value for buffer
    mov es, ax           ; Set ES to 0x3000
    mov ax, 0x4F01       ; VBE function: Get mode information
    mov cx, 0x118        ; Mode number (1024x768x32bpp)
    xor di, di           ; Offset within segment
    int 0x10             ; Call BIOS interrupt
    cmp ax, 0x004F       ; AX == 0x004F indicates success
    jne fail              ; Check for failure (carry flag set)

.load_protected_mode:
    cli
    lgdt[gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp 0x08:load32

%include "ata.asm"

fail:
    ; Indicate failure (e.g., blinking or halting system)
    mov ah, 0x0E         ; Teletype output function
    mov al, 'E'          ; Output 'E' for error
    int 0x10             ; BIOS interrupt
    jmp $$

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
    mov ecx, 200 ; sectors number
    mov edi, 0x0100000 ; load into
    call ata_lba_read

    .enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al

    mov ebx, 0
    mov eax, 1337

    jmp 0x08:0x0100000

[BITS 16]
%include "memmap.asm"
%include "gdt.asm"

times 446 - ($ - $$) db 0
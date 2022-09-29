ORG 0x7c00
BITS 16

jmp short setup ; jump over BIOS Parameter Block's space
nop

; FAT 16 Header
OEMIdentifier        db 'GIT OS  '
BytesPerSector       dw 0x200 ; 512 bytes per sector
SectorsPerCluster    db 0x80
ReservedSectors      dw 200 ; TODO: Shrink it down and move kernel to filesystem
FATCopies            db 0x02
RootDirEntries       dw 0x40
NumSectors           dw 0x00
MediaType            db 0xF8
SectorsPerFat        dw 0x100
SectorsPerTrack      dw 0x20
NumberOfHeads        dw 0x40
HiddenSectors        dd 0x00
SectorsBig           dd 0x773594

; Extended BPB (FAT 16)
DriveNumber          db 0x80
WinNTBit             db 0x00
Signature            db 0x29
VolumeID             dd 0xD105
VolumeIDString       db 'GIT OS BOOT'
SystemIDString       db 'FAT16   '


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
    call get_memory_map ; Load RAM info into 0x500
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
    .setup_pm_registers:
    mov ax, 0x10
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
%include "./src/boot/memmap.asm"
%include "./src/boot/gdt.asm"
; Aligning to 512 bytes
times 510 - ($ - $$) db 0
db 0x55
db 0xaa
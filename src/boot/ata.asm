[BITS 32]
; Simple ATA drive read driver
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
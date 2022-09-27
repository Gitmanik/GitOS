[BITS 16]
; https://forum.osdev.org/viewtopic.php?f=1&t=8683
; Requires 16-bit Real Mode and "newer" BIOSes to work
get_memory_map:
    push ds
    pop es
    mov di, 0x500
    xor ebx, ebx
.get_memory_map_loop:
    mov eax, 0x0000e820
    mov ecx, 0x20
    mov edx, 0x534d4150
    int 0x15
    jc .get_memory_map_end

    cmp eax, 0x534d4150
    jne .get_memory_map_end

    add di, 20
    cmp ebx, 0
    jne .get_memory_map_loop

.get_memory_map_end:
    clc ; Clear carry 
    ret
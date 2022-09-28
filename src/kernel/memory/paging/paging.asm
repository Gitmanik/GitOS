[BITS 32]

;Enables paging bit in cr0 register
global paging_enable
paging_enable:
    push ebp
    mov ebp, esp
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    pop ebp
    ret
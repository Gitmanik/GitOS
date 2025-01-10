[BITS 32]

global _start
section .crt0
extern main
_start:
    call main
    ret
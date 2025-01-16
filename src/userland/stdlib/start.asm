[BITS 32]

global _start
section .crt0
extern main
extern crt0
_start:
    call crt0
    ret
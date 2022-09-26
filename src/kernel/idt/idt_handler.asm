extern int21h_handler
extern no_int_handler

global int21h
int21h:
    cli
    pushad
    call int21h_handler
    popad
    sti
    iret

global no_int
no_int:
    cli
    pushad
    call no_int_handler
    popad
    sti
    iret
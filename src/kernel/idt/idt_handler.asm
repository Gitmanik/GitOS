extern int21h_handler
extern ignore_int_handler

global int21h
int21h:
    cli
    pushad
    call int21h_handler
    popad
    sti
    iret

global ignore_int
ignore_int:
    cli
    pushad
    call ignore_int_handler
    popad
    sti
    iret
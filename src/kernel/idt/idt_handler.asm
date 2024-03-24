extern int21h_handler
extern ignore_int_handler

global int21h
int21h:
    pushad
    call int21h_handler
    popad
    iret

global ignore_int
ignore_int:
    pushad
    call ignore_int_handler
    popad
    iret
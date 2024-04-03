extern idt_Handler

%macro macro_int 2
    global handler_int%1
    handler_int%1:
        ; Already pushed:
        ; uint32_t ip
        ; uint32_t cs
        ; uint32_t flags
        ; uint32_t sp
        ; uint32_t ss
        pushad
        %if %2==0
            push dword 0
        %endif
        push esp ; struct interrupt_frame*
        push dword %1 ; int int_no
        call idt_Handler
        add esp, 8
        %if %2==0
            add esp,4
        %endif
        popad
        iret
%endmacro

macro_int 0, 0
macro_int 1, 0
macro_int 2, 0
macro_int 3, 0
macro_int 4, 0
macro_int 5, 0
macro_int 6, 0
macro_int 7, 0
macro_int 8, 1
macro_int 9, 0
macro_int 10, 1
macro_int 11, 1
macro_int 12, 1
macro_int 13, 1
macro_int 14, 1
macro_int 15, 0
macro_int 16, 0
macro_int 17, 1
macro_int 18, 0
macro_int 19, 0
macro_int 20, 0
macro_int 21, 1
macro_int 22, 0
macro_int 23, 0
macro_int 24, 0
macro_int 25, 0
macro_int 26, 0
macro_int 27, 0
macro_int 28, 0
macro_int 29, 1
macro_int 30, 1
macro_int 31, 0

%assign i 32
%rep 224
    macro_int i, 0
%assign i i+1
%endrep

section .data

%macro interrupt_array_entry 1
    dd handler_int%1
%endmacro

global interrupt_pointer_table
interrupt_pointer_table:
%assign i 0
%rep 256
    interrupt_array_entry i
    %assign i i+1
%endrep

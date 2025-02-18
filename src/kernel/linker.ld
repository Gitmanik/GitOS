ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)
SECTIONS
{
    . = 1M;
    kernel_start = .;
    .text : ALIGN(4096)
    {
        *(.kernelstart)
        *(.text)
    }

    .rodata : ALIGN(4096)
    {
        *(.rodata)
    }
    .data : ALIGN(4096)
    {
        *(.data)
    }

    .bss : ALIGN(4096)
    {
        *(COMMON)
        *(.bss)
    }

    .stack (NOLOAD) : ALIGN(16) {
      stack_bottom = .;
      . = . + 16384;
      stack_top = .;
    }

    kernel_end = .;
}
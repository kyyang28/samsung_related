ENTRY(_start)
SECTIONS {
    . = 0x30000000;

    . = ALIGN(4);
    .text :
    {
        start.o(.text)
        init.o(.text)
        lowlevel_init.o(.text)
        nand.o(.text)
        *(.text)
    }

    . = ALIGN(4);
    .rodata : { *(.rodata) }

    . = ALIGN(4);
    .data : { *(.data) }

    . = ALIGN(4);
    __bss_start = .;
    .bss : { *(.bss) *(COMMON) }
    __bss_end = .;
}



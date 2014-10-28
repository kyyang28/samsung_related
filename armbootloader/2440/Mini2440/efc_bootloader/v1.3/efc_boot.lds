
SECTIONS {
    . = 0x33F80000;

    .text : 
    { 
        init/start.o             (.text)
        init/lowlevel_init.o     (.text)
        nand.o              (.text)
        init/boot_init.o         (.text)
        *(.text) 
    }

    . = ALIGN(4);
    .rodata : { *(.rodata) }

    . = ALIGN(4);
    .data : { *(.data) }

    . = ALIGN(4);
    __bss_start = .;
    .bss : { *(.bss) *(COMMON) }
    __bss_end = .;
}


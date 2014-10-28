SECTIONS 
{
    . = 0x50000000;
    
	.text : 
	{ 
        start.o (.text)
		clock.o (.text)
		ddr.o   (.text)
		irq.o	(.text)
        *(.text)
    } 
    
    . = ALIGN(4);
    .rodata : { *(.rodata) }
    
    . = ALIGN(4);
	.data : { *(.data) }

    . = ALIGN(4);	
    __bss_start = .;
	.bss : { *(.bss) }
	__bss_end = .;   
}


SECTIONS
{
	. = 0x30000000;

	. = ALIGN(4);
	.text	:	
	{
		start.o				(.text)
		clock_asm_init.o	(.text)
		sdram_asm_init.o	(.text)
		nand_ops.o			(.text)
		*(.text)
	}

	. = ALIGN(4);
	.rodata		:	{ *(.rodata) }

	. = ALIGN(4);
	.data		:	{ *(.data) }

	. = ALIGN(4);
	__bss_start = .;
	.bss	:	{ *(.bss) }
	_end = .;
}


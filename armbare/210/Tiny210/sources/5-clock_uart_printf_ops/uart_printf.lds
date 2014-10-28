SECTIONS
{
	. = 0xD0020010;

	. = ALIGN(4);
	.text : {
		start.o
		*(.text)
	}

	. = ALIGN(4);
	.rodata :	{ *(.rodata) }

	. = ALIGN(4);
	.data	:	{ *(.data) }

	. = ALIGN(4);
	__bss_start = .;
	.bss	:	{ *(.bss) }
	__bss_end = .;
}


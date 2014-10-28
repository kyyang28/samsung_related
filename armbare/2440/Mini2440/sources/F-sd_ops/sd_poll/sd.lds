SECTIONS
{
	. = 0x30000000;

	. = ALIGN(4);
	.text		:	{ *(.text) }

	. = ALIGN(4);
	.rodata		:	{ *(.rodata) }

	. = ALIGN(4);
	.data		:	{ *(.data) }

	__bss_start = .;
	.bss		:	{ *(.bss) }
	_end = .;
}


SECTIONS {
	. = 0x52000000;

	.text ALIGN(4) :
	{
		start.o (.text)
		*(.text)
	}

	.rodata ALIGN(4) :
	{
		*(.rodata)
	}

	.data ALIGN(4) :
	{
		*(.data)
	}

	__bss_start = .;
	.bss ALIGN(4) :
	{
		*(.bss)
		*(COMMON)
	}
	__bss_end = .;
}


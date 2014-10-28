SECTIONS {
	. = 0x50000000;

	.text :
	{
		start.o
		*(.text)
	}

	.data :
	{
		*(.data)
	}

	__bss_start = .;
	.bss :
	{
		*(.bss)
	}
	__bss_end = .;
}


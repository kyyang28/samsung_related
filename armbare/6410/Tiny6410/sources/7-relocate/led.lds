SECTIONS {
	. = 0x1000;

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


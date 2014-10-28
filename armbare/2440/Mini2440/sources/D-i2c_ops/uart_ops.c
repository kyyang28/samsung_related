

#include "s3c2440-regs.h"

int uart_init_ll(void)
{
	GPHCON |= (0x02<<4 | 0x02<<6);
	
	ULCON0 = 0x3;	/* 11 = 8-bits */
	UCON0 = 0x5;	/* 01 = Interrupt request or polling mode */
	
#ifdef ENABLE_UART_FIFO
	UFCON0 = 0x1;
#endif
	
	UBRDIV0 = (int)UBRDIV0_VALS;
	return 0;
}

#ifdef ENABLE_UART_FIFO
void serial_putc_ll(u8 c)
{
	while (UFSTAT0 & (1<<14));
	UTXH0 = c;
}

u8 serial_getc_ll(void)
{
	while (!(UFSTAT0 & 0x7f));
	return URXH0;
}
#else
void serial_putc_ll(u8 c)
{
	while (!(UTRSTAT0 & (1<<2)));
	UTXH0 = c;
}


u8 serial_getc_ll(void)
{
	while (!(UTRSTAT0 & (1<<0)));
	return URXH0;
}
#endif

void serial_puts_ll(char *str)
{
	while (*str) {
		serial_putc_ll(*str++);
	}
}



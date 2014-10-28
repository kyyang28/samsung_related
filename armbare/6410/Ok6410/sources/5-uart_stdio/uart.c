
#include "s3c6410_regs.h"

#ifdef ENABLE_UART_FIFO
unsigned char uart_getc(void)
{
	while (!(UFSTAT0 & 0x7F));
	return URXH0;
}

void uart_putc(unsigned char c)
{
	while (UFSTAT0 & (1<<14));
	UTXH0 = c;
}
#else
unsigned char uart_getc(void)
{
	while (!(UTRSTAT0 & (1<<0)));
	return URXH0;
}

void uart_putc(unsigned char c)
{
	while (!(UTRSTAT0 & (1<<2)));
	UTXH0 = c;
}
#endif


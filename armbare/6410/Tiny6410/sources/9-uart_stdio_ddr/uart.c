
#include "s3c6410_regs.h"

void uart_init(void)
{
	/* Setup the GPIO for RXD and TXD */
	GPACON &= ~(0xFF);
	GPACON |= 0x22;
	
	/* The number of data bits = 11(8-bit) */
	ULCON0 = 0x3;
	
	/* 
	 *	Clock Selection [11:10] 10 = PCLK : DIV_VAL = (PCLK / (bps x 16) ) - 1 
	 *	Transmit/Receive Modes: 01 = Interrupt request or polling mode 	
	 */
	UCON0 = 0x805;
	
	/* FIFO Enable: 1 = Enable */
#ifdef ENABLE_UART_FIFO
	UFCON0 = 0x1;
#endif

	/* Baudrate divisor */
	UBRDIV0 = 35;	/* PCLK = 67MHz, Baudrate = 115200 */
	
	/* The number of 1's in UBRDIV0 */
	UDIVSLOT0 = 0x0080;	/* The number of 1's is 1 */
}


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


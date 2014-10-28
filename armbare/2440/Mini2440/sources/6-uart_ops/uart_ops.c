

/* GPIO for UART */
#define GPHCON								(*(volatile unsigned long *)0x56000070)
#define GPHDAT								(*(volatile unsigned long *)0x56000074)
#define GPHUP								(*(volatile unsigned long *)0x56000078)

/* UART registers */
#define ULCON0								(*(volatile unsigned long *)0x50000000)
#define UCON0								(*(volatile unsigned long *)0x50000004)
#define UFCON0								(*(volatile unsigned long *)0x50000008)
#define UTRSTAT0							(*(volatile unsigned long *)0x50000010)
#define UFSTAT0								(*(volatile unsigned long *)0x50000018)
#define UTXH0								(*(volatile unsigned char *)0x50000020)
#define URXH0								(*(volatile unsigned char *)0x50000024)
#define UBRDIV0								(*(volatile unsigned long *)0x50000028)

#define PCLK								(50000000)
#define BAUDRATE							(115200)
#define UBRDIV0_VALS						((PCLK / (BAUDRATE * 16)) - 1)

void uart_init(void)
{
	GPHCON |= (0x02<<4 | 0x02<<6);
	
	ULCON0 = 0x3;	/* 11 = 8-bits */
	UCON0 = 0x5;	/* 01 = Interrupt request or polling mode */
	
#ifdef ENABLE_UART_FIFO
	UFCON0 = 0x1;
#endif
	
	/* PCLK = 50MHZ, Baudrate = 115200 */
	UBRDIV0 = (int)UBRDIV0_VALS;
}

#ifdef ENABLE_UART_FIFO
void serial_putc(unsigned char c)
{
	while (UFSTAT0 & (1<<14));
	UTXH0 = c;
}

unsigned char serial_getc(void)
{
	while (!(UFSTAT0 & 0x7f));
	return URXH0;
}
#else
void serial_putc(unsigned char c)
{
	while (!(UTRSTAT0 & (1<<2)));
	UTXH0 = c;
}


unsigned char serial_getc(void)
{
	while (!(UTRSTAT0 & (1<<0)));
	return URXH0;
}
#endif


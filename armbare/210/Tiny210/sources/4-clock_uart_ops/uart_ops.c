
/* GPIO for Uart0 */
#define GPA0CON								(*(volatile unsigned long *)0xE0200000)
#define GPA0DAT								(*(volatile unsigned long *)0xE0200004)

/* Uart Controller Registers */
#define ULCON0								(*(volatile unsigned long *)0xE2900000)
#define UCON0								(*(volatile unsigned long *)0xE2900004)
#define UFCON0								(*(volatile unsigned long *)0xE2900008)
#define UTRSTAT0							(*(volatile unsigned long *)0xE2900010)
#define UFSTAT0								(*(volatile unsigned long *)0xE2900018)
#define UTXH0								(*(volatile unsigned char *)0xE2900020)
#define URXH0								(*(volatile unsigned char *)0xE2900024)
#define UBRDIV0								(*(volatile unsigned long *)0xE2900028)
#define UDIVSLOT0							(*(volatile unsigned long *)0xE290002C)

#define PCLK								(66000000)
#define BAUD_RATE							(115200)
#define UBRDIV0_VAL							((PCLK / (BAUD_RATE * 16)) - 1)

int uart_init_ll(void)
{
	/* Setup the GPA0 to uart functions */
	GPA0CON |= 0x22;

	/* Setup the ULCON0 Register */
	ULCON0 = 0x3;		/* Data bits = 8-bit */

	/* Setup the UCON0 Register */
	UCON0 = 0x5;

	/* Setup the UBRDIV0 Register */
	UBRDIV0 = (int)UBRDIV0_VAL; 

	/* Setup the UDIVSLOT0 Register */
	UDIVSLOT0 = 0xDFDD;

	return 0;
}

void uart_putc(unsigned char ch)
{
	while (!(UTRSTAT0 & (1<<2)));
	UTXH0 = ch;
}

unsigned char uart_getc(void)
{
	while (!(UTRSTAT0 & (1<<0)));
	return URXH0;
}

void uart_puts(char *str)
{
	while (*str != '\0')
		uart_putc(*str++);
}

void uart_test_ll(void)
{
	unsigned char recv;

	uart_puts("\r\n");
	uart_puts("Welcome to uart testing program!");
	uart_puts("\r\n\r\n");

	while (1) {
		recv = uart_getc();

		if (recv == '\r') {
			uart_putc('\r');
			uart_putc('\n');
		}else if (recv == '\b') {
			uart_putc('\b');
			uart_putc(' ');
			uart_putc('\b');
		}else {
			uart_putc(recv);
		}
	}
}


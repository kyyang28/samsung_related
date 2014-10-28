
#include "stdio.h"
#include "uart_ops.h"

int uart_init_ll(void)
{
#ifdef UART0
	/* Setup the GPA0 to uart functions */
	GPA0CON |= 0x22;

	/* Setup the ULCON0 Register */
	ULCON0 = 0x3;		/* Data bits = 8-bit */

	/* Setup the UCON0 Register */
	UCON0 = 0x5;

	/* Setup the UBRDIV0 Register */
	UBRDIV0 = (int)UBRDIV0_VAL; 

	/* Setup the UDIVSLOT0 Register */
	UDIVSLOT0 = 0xDDDD;
#endif

#ifdef UART3
	/* Setup the GPA0 to uart functions */
	GPA1CON |= 0x2200;

	/* Setup the ULCON0 Register */
	ULCON3 = 0x3;		/* Data bits = 8-bit */

	/* Setup the UCON0 Register */
	UCON3 = 0x5;

	/* Setup the UBRDIV0 Register */
	UBRDIV3 = (int)UBRDIV0_VAL; 

	/* Setup the UDIVSLOT0 Register */
	UDIVSLOT3 = 0xDDDD;
#endif

	return 0;
}

void uart_putc(unsigned char ch)
{
#ifdef UART0
	while (!(UTRSTAT0 & (1<<2)));
#endif

#ifdef UART3
	while (!(UTRSTAT3 & (1<<2)));
#endif

	UTXH0 = ch;
}

unsigned char uart_getc(void)
{
#ifdef UART0
	while (!(UTRSTAT0 & (1<<0)));
#endif

#ifdef UART3
	while (!(UTRSTAT3 & (1<<0)));
#endif

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

	printf("\r\n");
	printf("Welcome to uart testing program!");
	printf("\r\n\r\n");

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


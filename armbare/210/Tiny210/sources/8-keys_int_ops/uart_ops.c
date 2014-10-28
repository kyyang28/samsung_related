
#include "stdio.h"

/* GPIO for Uart0 */
#define GPA0CON								(*(volatile unsigned long *)0xE0200000)
#define GPA0DAT								(*(volatile unsigned long *)0xE0200004)

/* GPIO for Uart3 */
#define GPA1CON								(*(volatile unsigned long *)0xE0200020)
#define GPA1DAT								(*(volatile unsigned long *)0xE0200024)

/* Uart0 Controller Registers */
#define ULCON0								(*(volatile unsigned long *)0xE2900000)
#define UCON0								(*(volatile unsigned long *)0xE2900004)
#define UFCON0								(*(volatile unsigned long *)0xE2900008)
#define UTRSTAT0							(*(volatile unsigned long *)0xE2900010)
#define UFSTAT0								(*(volatile unsigned long *)0xE2900018)
#define UTXH0								(*(volatile unsigned char *)0xE2900020)
#define URXH0								(*(volatile unsigned char *)0xE2900024)
#define UBRDIV0								(*(volatile unsigned long *)0xE2900028)
#define UDIVSLOT0							(*(volatile unsigned long *)0xE290002C)

/* Uart3 Controller Registers */
#define ULCON3								(*(volatile unsigned long *)0xE2900C00)
#define UCON3								(*(volatile unsigned long *)0xE2900C04)
#define UFCON3								(*(volatile unsigned long *)0xE2900C08)
#define UTRSTAT3							(*(volatile unsigned long *)0xE2900C10)
#define UFSTAT3								(*(volatile unsigned long *)0xE2900C18)
#define UTXH3								(*(volatile unsigned char *)0xE2900C20)
#define URXH3								(*(volatile unsigned char *)0xE2900C24)
#define UBRDIV3								(*(volatile unsigned long *)0xE2900C28)
#define UDIVSLOT3							(*(volatile unsigned long *)0xE2900C2C)

#define PCLK								(66000000)
#define BAUD_RATE							(115200)
#define UBRDIV0_VAL							((PCLK / (BAUD_RATE * 16)) - 1)

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


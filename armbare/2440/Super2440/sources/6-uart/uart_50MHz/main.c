
#include "uart.h"

#define COM0		(0)
#define COM1 		(1)
#define COM2		(2)


static inline void DELAY (unsigned long ticks)
{
	__asm__ volatile (
	  "1:\n"
	  "subs %0, %1, #1\n"
	  "bne 1b":"=r" (ticks):"0" (ticks));
}


int mymain(void)
{
	int i, j;
	unsigned char recv;
	
	uart_init(COM0);

	static char title[32] = "Welcome to UART test program!\r\n";
	static char alphabet[27] = "abcdefghijklmnopqrstuvwxyz";
	
	for(i = 0; i < 32; ++i)
		uart_putchar(COM0, title[i]);
	
	for(j = 0; j < 26; ++j) {
		uart_putchar(COM0, alphabet[j]);
		DELAY(100000);
	}
	
	uart_putchar(COM0, '\r');
	uart_putchar(COM0, '\n');

	while(1) {
		recv = uart_getchar(COM0);
		if(recv == '\r') {
			uart_putchar(COM0, '\r');
			uart_putchar(COM0, '\n');
		}else if(recv == '\b') {
			uart_putchar(COM0, '\b');
			uart_putchar(COM0, ' ');
			uart_putchar(COM0, '\b');
		}else
			uart_putchar(COM0, recv);
	}

	return 0;
}



#include "uart.h"

int myuart(void)
{
	unsigned char recv;

	while (1) {
		recv = uart_getc();
		if (recv == '\r') {
			uart_putc('\r');
			uart_putc('\n');
		}else if (recv == '\b') {
			uart_putc('\b');
			uart_putc(' ');
			uart_putc('\b');
		}else 
			uart_putc(recv);
	}
	
	return 0;
}


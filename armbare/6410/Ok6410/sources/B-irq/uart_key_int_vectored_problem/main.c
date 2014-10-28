

extern void uart_putc(char c);
extern unsigned char uart_getc(void);

int myuartint(void)
{
#if 0
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
#else
	while (1);
#endif
	return 0;
}


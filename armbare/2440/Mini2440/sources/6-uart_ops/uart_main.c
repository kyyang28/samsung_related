

extern void uart_init(void);
extern void serial_putc(unsigned char c);
extern unsigned char serial_getc(void);

int uart_main(void)
{
	unsigned char recv;

	uart_init();

	while (1) {
		recv = serial_getc();

		if (recv == '\r') {
			serial_putc('\r');
			serial_putc('\n');
		}else if (recv == '\b') {
			serial_putc('\b');
			serial_putc(' ');
			serial_putc('\b');
		}else {
			serial_putc(recv);
		}
	}
	
	return 0;
}




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

void uart_init_ll(void)
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
void serial_putc_ll(unsigned char c)
{
	while (UFSTAT0 & (1<<14));
	UTXH0 = c;
}

unsigned char serial_getc_ll(void)
{
	while (!(UFSTAT0 & 0x7f));
	return URXH0;
}
#else
void serial_putc_ll(unsigned char c)
{
	while (!(UTRSTAT0 & (1<<2)));
	UTXH0 = c;
}


unsigned char serial_getc_ll(void)
{
	while (!(UTRSTAT0 & (1<<0)));
	return URXH0;
}
#endif

void serial_puts_ll(char *str)
{
	while (*str) {
		serial_putc_ll(*str++);
	}
}


/* ############################################################################ */

void serial_putchar_hex(char c)
{
	char * hex = "0123456789ABCDEF";
		
	serial_putc_ll(hex[(c>>4) & 0x0F]);
	serial_putc_ll(hex[(c>>0) & 0x0F]);
}

void serial_putint_hex(int a)
{
	serial_putchar_hex( (a>>24) & 0xFF );
	serial_putchar_hex( (a>>16) & 0xFF );
	serial_putchar_hex( (a>>8)	& 0xFF );
	serial_putchar_hex( (a>>0)	& 0xFF );
}

char * itoa(int a, char * buf)
{
	int num = a;
	int i = 0;
	int len = 0;

	do {
		buf[i++] = num % 10 + '0';
		num /= 10;		
	} while (num);

	buf[i] = '\0';

	len = i;
	for (i = 0; i < len/2; i++) {
		char tmp;
		tmp = buf[i];
		buf[i] = buf[len-i-1];
		buf[len-i-1] = tmp;
	}
	
	return buf;	
}

typedef int * va_list;
#define va_start(ap, A)		(ap = (int *)&(A) + 1)
#define va_arg(ap, T)		(*(T *)ap++)
#define va_end(ap)		((void)0)

int serial_printf(const char * format, ...)
{
	char c;	
	va_list ap;
	
	va_start(ap, format);

	while ((c = *format++) != '\0') {
		switch (c) {
			case '%':
				c = *format++;
				
				switch (c) {
					char ch;
					char *p;
					int a;
					char buf[100];

					case 'c':
						ch = va_arg(ap, int);
						serial_putc_ll(ch);
						break;
					
					case 's':
						p = va_arg(ap, char *);
						serial_puts_ll(p);
						break;					

					case 'x':
						a = va_arg(ap, int);
						serial_putint_hex(a);
						break;		
					
					case 'd':
						a = va_arg(ap, int);
						itoa(a, buf);
						serial_puts_ll(buf);
						break;	

					default:
						break;
				}
				break;		
			
			default:
				serial_putc_ll(c);
				break;
		}
	}

	return 0;
}


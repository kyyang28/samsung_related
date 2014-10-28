

extern void serial_putc_ll(unsigned char c);
extern void serial_puts_ll(char *str);

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



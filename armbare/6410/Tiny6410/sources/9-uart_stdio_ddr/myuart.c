
extern int printf(const char *fmt, ...);
extern int scanf(const char * fmt, ...);

extern void uart_putc(unsigned char);

int myuart(void)
{
	int num1, num2;

	while (1) {
		printf("Please enter two number: \r\n");
		scanf("%d %d", &num1, &num2);
		printf("\r\n");
		printf("The sum of %d,%d is: %d\r\n", num1, num2, num1+num2);
	}
	
	return 0;
}


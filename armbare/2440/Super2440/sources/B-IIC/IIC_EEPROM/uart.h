
#ifndef __UART_H__
#define	__UART_H__

void uart_init(int COM);
void uart_putchar(int COM, unsigned char ch);
unsigned char uart_getchar(int COM);

#endif


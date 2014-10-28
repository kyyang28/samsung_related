#ifndef __UART_OPS_H
#define __UART_OPS_H

void uart_init_ll(void);
void uart_putc(unsigned char ch);
unsigned char uart_getc(void);
void uart_puts(char *str);
void uart_test_ll(void);

#endif


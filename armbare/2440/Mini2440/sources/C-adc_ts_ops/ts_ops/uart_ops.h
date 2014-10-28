#ifndef __UART_OPS_H
#define __UART_OPS_H

void uart_init_ll(void);
void serial_putc_ll(unsigned char c);
unsigned char serial_getc_ll(void);
void serial_puts_ll(char *str);

#endif


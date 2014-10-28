#ifndef __UART_OPS_H
#define __UART_OPS_H

#include "s3c2440-regs.h"

int uart_init_ll(void);
void serial_putc_ll(u8 c);
u8 serial_getc_ll(void);
void serial_puts_ll(char *str);

#endif


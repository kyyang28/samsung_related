
#include "uart_ops.h"

int clock_uart_main(void)
{
	/* Uart init */
	uart_init_ll();

	/* Uart testing program */
	uart_test_ll();

	return 0;
}



#include "uart_ops.h"
#include "ts_ops.h"

int ts_main(void)
{
	/* Uart initialization */
	uart_init_ll();
	
	/* Touch Screen Test */
	ts_test();

	return 0;
}


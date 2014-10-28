
#include "stdio.h"
#include "uart_ops.h"
#include "adc_ops.h"


int adc_main(void)
{
	/* Uart init */
	uart_init_ll();

	/* Adc test */
	adc_test_ll();

	return 0;
}


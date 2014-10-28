
#include <stdio.h>
#include "sd_ops.h"
#include "uart_ops.h"

int sd_main(void)
{
	/* Uart initialization */
	uart_init_ll();

	/* Test SD program */
    Test_SDI();

	while (1);
	return 0;
}


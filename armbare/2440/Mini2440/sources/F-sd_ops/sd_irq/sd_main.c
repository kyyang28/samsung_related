
#include <stdio.h>
#include "sd_ops.h"
#include "sd_irq.h"
#include "uart_ops.h"

int sd_main(void)
{
	/* Uart initialization */
	uart_init_ll();

	/* SD interrupt initialization */
	sd_int_init();

	/* Test SD program */
    Test_SDI();

	while (1);
	return 0;
}


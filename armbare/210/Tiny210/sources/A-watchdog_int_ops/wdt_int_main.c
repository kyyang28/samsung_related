
#include "stdio.h"
#include "leds_ops.h"
#include "uart_ops.h"
#include "interrupt.h"

int wdt_int_main(void)
{
	/* Leds init */
	leds_init_ll();

	/* Uart init */
	uart_init_ll();

	/* Interrupt init */
	interrupt_init_ll();

	while (1); 

	return 0;
}


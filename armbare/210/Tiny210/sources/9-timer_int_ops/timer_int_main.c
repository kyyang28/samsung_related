
#include "stdio.h"
#include "leds_ops.h"
#include "uart_ops.h"
#include "interrupt.h"

int timer_int_main(void)
{
	/* Leds init */
	leds_init_ll();

	/* Uart init */
	uart_init_ll();

	printf("Hello world!\r\n");

	/* Interrupt init */
	interrupt_init_ll();

	while (1) {
		printf("WTF!\r\n");
		break;
	}

	return 0;
}


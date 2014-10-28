
#include "leds_ops.h"
#include "uart_ops.h"

int ddr2_leds_uart_main(void)
{
	/* Leds init */
	leds_init();

	/* Uart init */
	uart_init_ll();

#ifdef LEDS_TEST
	/* Leds test */
	leds_run();
#endif

#ifdef UART_TEST
	/* Uart test */
	uart_test_ll();
#endif

	return 0;
}


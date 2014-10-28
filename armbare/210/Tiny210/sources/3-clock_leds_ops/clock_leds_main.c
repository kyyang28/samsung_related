
#include "leds_ops.h"

int clock_leds_main(void)
{
	/* Leds init */
	leds_init();

	/* Leds test */
	leds_run();

	return 0;
}


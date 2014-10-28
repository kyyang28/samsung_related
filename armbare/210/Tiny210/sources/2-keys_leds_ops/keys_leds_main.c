
#include "leds_ops.h"
#include "keys_ops.h"

int keys_leds_main(void)
{
	/* Leds init */
	leds_init();

	/* Keys init */
	keys_init();

	/* Keys Leds Testing Program */
	keys_leds_test();

	return 0;
}


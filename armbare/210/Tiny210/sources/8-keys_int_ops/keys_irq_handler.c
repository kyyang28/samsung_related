
#include "stdio.h"
#include "leds_ops.h"
#include "interrupt.h"

void keys_irq_handler(void)
{
	printf("Entering keys_irq_handler ...\r\n");

	/* K1 */
	if (EXT_INT_2_PEND & (1<<0)) {
		if (!(GPH2DAT & (1<<0))) {
			printf("K1 is pressed down!\r\n");
			leds_on(1);
		}else {
			printf("K1 is released!\r\n");
			leds_off(1);
		}
	}

	/* K2 */
	if (EXT_INT_2_PEND & (1<<1)) {
		if (!(GPH2DAT & (1<<1))) {
			printf("K2 is pressed down!\r\n");
			leds_on(2);
		}else {
			printf("K2 is released!\r\n");
			leds_off(2);
		}
	}

	/* K3 */
	if (EXT_INT_2_PEND & (1<<2)) {
		if (!(GPH2DAT & (1<<2))) {
			printf("K3 is pressed down!\r\n");
			leds_on(3);
		}else {
			printf("K3 is released!\r\n");
			leds_off(3);
		}
	}
	
	/* K4 */
	if (EXT_INT_2_PEND & (1<<3)) {
		if (!(GPH2DAT & (1<<3))) {
			printf("K4 is pressed down!\r\n");
			leds_on(4);
		}else {
			printf("K4 is released!\r\n");
			leds_off(4);
		}
	}

	/* K5 */
	if (EXT_INT_3_PEND & (1<<0)) {
		if (!(GPH3DAT & (1<<0))) {
			printf("K5 is pressed down!\r\n");
			leds_on(1);
			leds_on(2);
		}else {
			printf("K5 is released!\r\n");
			leds_off(1);
			leds_off(2);
		}
	}

	/* K6 */
	if (EXT_INT_3_PEND & (1<<1)) {
		if (!(GPH3DAT & (1<<1))) {
			printf("K6 is pressed down!\r\n");
			leds_on(3);
			leds_on(4);
		}else {
			printf("K6 is released!\r\n");
			leds_off(3);
			leds_off(4);
		}
	}

	/* K7 */
	if (EXT_INT_3_PEND & (1<<2)) {
		if (!(GPH3DAT & (1<<2))) {
			printf("K7 is pressed down!\r\n");
			leds_on(1);
			leds_on(3);
		}else {
			printf("K7 is released!\r\n");
			leds_off(1);
			leds_off(3);
		}
	}

	/* K8 */
	if (EXT_INT_3_PEND & (1<<3)) {
		if (!(GPH3DAT & (1<<3))) {
			printf("K8 is pressed down!\r\n");
			leds_on(2);
			leds_on(4);
		}else {
			printf("K8 is released!\r\n");
			leds_off(2);
			leds_off(4);
		}
	}
	
	EXT_INT_2_PEND = (1<<0 | 1<<1 | 1<<2 | 1<<3);
	EXT_INT_3_PEND = (1<<0 | 1<<1 | 1<<2 | 1<<3);

	clearVecAddress();
}


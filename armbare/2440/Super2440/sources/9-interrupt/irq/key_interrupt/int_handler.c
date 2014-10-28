
#include "s3c2440.h"
#include "led.h"


void Interrupt_handler(void)
{
	unsigned long offset = INTOFFSET;

	/* 
	 * EINT1(K2), EINT3(K3), EINT5(K4)
	 * lights on led 1, led 2, led 3 respectively
	 *
	 * EINT0(K5), EINT2(K6), EINT4(K7)
	 * lights off led 1, led 2, led 3 respectively
	 */
	switch(offset) {
		case 0:
			led_off(1);
			break;

		case 2:
			led_off(2);
			break;
		
		case 1:
			led_on(1);
			break;
			
		case 3:
			led_on(2);
			break;
			
		case 4:
			if(EINTPEND & (1<<4))
				led_off(3);

			if(EINTPEND & (1<<5))
				led_on(3);
			break;
			
		default:
			break;
	}
	
	if(offset == 4)
		EINTPEND = (1<<4 | 1<<5);
	
	SRCPND = 1<<offset;
	INTPND = 1<<offset;
}

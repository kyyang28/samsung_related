

#include "s3c2440-regs.h"

int leds_init_ll(void)
{
	GPBCON = (0x1<<16) | (0x1<<14) | (0x1<<12) | (0x1<<10);
	GPBDAT |= 0xF<<5;
	return 0;
}

void leds_on(int num)
{
	switch (num) {
	case 1:
		GPBDAT &= ~(1<<5);
		break;

	case 2:
		GPBDAT &= ~(1<<6);
		break;

	case 3:
		GPBDAT &= ~(1<<7);
		break;

	case 4:
		GPBDAT &= ~(1<<8);
		break;
	}
}

void leds_off(int num)
{
	switch (num) {
	case 1:
		GPBDAT |= (1<<5);
		break;

	case 2:
		GPBDAT |= (1<<6);
		break;

	case 3:
		GPBDAT |= (1<<7);
		break;

	case 4:
		GPBDAT |= (1<<8);
		break;
	
	default:
		break;
	}
}


void leds_ops(void)
{
	leds_on(1);
	DELAY(300000);
	leds_off(1);
	DELAY(300000);
}


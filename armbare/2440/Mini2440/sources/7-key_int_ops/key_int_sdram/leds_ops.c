

#include "s3c2440-regs.h"


void leds_init(void)
{
	GPBCON = (0x1<<16) | (0x1<<14) | (0x1<<12) | (0x1<<10);
	GPBDAT = 0xF<<5;
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


static void delay(void)
{
	volatile int i;
	for (i = 0; i < 100000; ++i);
}

void leds_ops(void)
{
	unsigned int i = 0;

	while (1) {
		GPBDAT = ~(i<<8);
		if (i++ == 1)
			i = 0;
		delay();
	}
}


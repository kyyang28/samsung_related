
#include "s3c2440-regs.h"

void leds_init(void)
{
	GPBCON = (0x01<<16) | (0x01<<14) | (0x01<<12) | (0x01<<10);

	/* WARNING: Do not lights off all the leds */
	//GPBDAT = 0xf<<5;	
}

void leds_all_off(void)
{
	GPBDAT = 0xf<<5;
}

void leds_all_on(void)
{
	GPBDAT = ~(0xf<<5);
}


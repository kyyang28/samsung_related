

#include "s3c2440-regs.h"
#include "lib_ops.h"

int buzzer_init_ll(void)
{
	GPBCON |= 0x01;
	return 0;
}

void buzzer_on(void)
{
	GPBDAT |= 0x1;
}

void buzzer_off(void)
{
	GPBDAT &= ~(0x1);
}

void buzzer_ops(void)
{
	buzzer_on();
	delay(1000);
	buzzer_off();
	delay(1000);
}


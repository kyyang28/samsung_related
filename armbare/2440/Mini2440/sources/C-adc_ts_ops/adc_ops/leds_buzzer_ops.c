

#include "s3c2440-regs.h"

extern int serial_printf(const char * format, ...); 

void leds_buzzer_init_ll(void)
{
	/* Setup the GPB0,5,6,7,8 pins to be OUTPUT  */
	GPBCON = 0x1<<10 | 0x1<<12 | 0x1<<14 | 0x1<<16 | 0x1;
	GPBDAT = 0xF<<5;
	GPBDAT &= ~(0x1);
}

/* num is 1,2,3,4 */
void leds_on(int num)
{
	if (1 == num || 2 == num || 3 == num || 4 == num)
		GPBDAT &= ~(1<<(num+4));
	else
		serial_printf("\r\nWrong led number, please enter 1,2,3,4\r\n");
}

/* num is 1,2,3,4 */
void leds_off(int num)
{
	if (1 == num || 2 == num || 3 == num || 4 == num)
		GPBDAT |= (1<<(num+4));
	else
		serial_printf("\r\nWrong led number, please enter 1,2,3,4\r\n");
}

void leds_all_on(void)
{
	GPBDAT &= ~(0xf<<5);
}

void leds_all_off(void)
{
	GPBDAT |= (0xf<<5);
}

void buzzer_on(void)
{
	GPBDAT |= 0x1;
}

void buzzer_off(void)
{
	GPBDAT &= ~(0x1);
}


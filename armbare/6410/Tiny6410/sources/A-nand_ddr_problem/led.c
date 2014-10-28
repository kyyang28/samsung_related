
#include "s3c6410_regs.h"

#ifdef BOOT_FROM_NAND
void delay(void)
{
	volatile unsigned int i;
	for (i = 0; i < 50000; i++);
}
#else
void delay(int loop)
{
	volatile unsigned int i, j;
	for (i = 0; i < 0x100000; i++)
		for (j = 0; j < loop; j++)
			;
}
#endif


void led_init(void)
{
	/* 
	 *	GPM0 [3:0]		0001 = Output 
	 *	GPM1 [7:4]		0001 = Output 
	 *	GPM2 [11:8]		0001 = Output 
	 *	GPM3 [15:12]	0001 = Output
	 *	Setup to output pin
	 */
	GPMCON = 0x1111;

	/* light off all leds */
	GPMDAT = 0xF;
}

/* 
 *	led_on(1): 1st led on 
 *	led_on(2): 2nd led on 
 *	led_on(3): 3rd led on 
 *	led_on(4): 4th led on 
 */
void led_on(int num)
{
	GPMDAT &= ~(1<<(num-1));
}

/* 
 *	led_off(1): 1st led off 
 *	led_off(2): 2nd led off 
 *	led_off(3): 3rd led off 
 *	led_off(4): 4th led off 
 */
void led_off(int num)
{
	GPMDAT |= (1<<(num-1));
}

void led_run(void)
{
	volatile unsigned int i;
	while (1) {
		for (i = 0; i < 4; i++) {
			GPMDAT = ~(1<<i);
#ifdef BOOT_FROM_NAND
			delay();
#else
			delay(10);
#endif
		}
	}
}



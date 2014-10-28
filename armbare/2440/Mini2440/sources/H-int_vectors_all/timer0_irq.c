
#include "s3c2440-regs.h"
#include "int_irq_vectors.h"

void timer0_isr(void)
{
	GPBDAT = ~(GPBDAT & 0x1);
}

/*
 *	Timer input clock Frequency = PCLK / {prescaler value+1} / {divider value}
 *	{prescaler value} = 0~255
 *	{divider value} = 2, 4, 8, 16
 */
void timer0_int_init_ll(void)
{
	/* Step 1: Timer0 source */
	TCFG0 = 99;
	TCFG1 = 0x03;
	TCNTB0 = 31250;				/* PCLK = 50MHz, 1s = 31250 */
	//TCNTB0 = 7500;			/* PCLK = 12MHz, 1s = 7500 */
	//TCNTB0 = 3750;			/* PCLK = 12MHz, 0.5s = 7500/2 = 3750 */
	TCON |= (1<<1);
	TCON = (0x01<<3) | (0x01);
}


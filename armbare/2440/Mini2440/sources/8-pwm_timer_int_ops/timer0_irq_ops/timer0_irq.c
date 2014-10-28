
#include "s3c2440-regs.h"

/*
 *	Timer input clock Frequency = PCLK / {prescaler value+1} / {divider value}
 *	{prescaler value} = 0~255
 *	{divider value} = 2, 4, 8, 16
 */
void timer0_irq_init(void)
{
	/* Step 1: Timer0 source */
	TCFG0 = 99;
	TCFG1 = 0x03;
	TCNTB0 = 7500;		/* PCLK = 12MHz, 1s = 7500 */
	//TCNTB0 = 3750;			/* PCLK = 12MHz, 0.5s = 7500/2 = 3750 */
	TCON |= (1<<1);
	TCON = (0x01<<3) | (0x01);

	/* Step 2: Interrupt controller */
	INTMOD &= ~(1<<10);
	INTMSK &= ~(1<<10);
	
	/* Step 3: Enable CPSR I-bit */
	__asm__ (
		"mrs r0, cpsr\n"
		"and r0, r0, #~(1<<7)\n"
		"msr cpsr, r0\n"
	);
}

void timer0_isr(void)
{
	unsigned long intoft = INTOFFSET;

	if (10 == intoft) {
		GPBDAT = ~(GPBDAT & (0x1<<5));
	}

	SRCPND = 1<<intoft;
	INTPND = 1<<intoft;
}


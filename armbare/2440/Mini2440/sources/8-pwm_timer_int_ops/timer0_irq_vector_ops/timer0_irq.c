
#include "s3c2440-regs.h"

void (*isr[60])(void);

void dummy_isr(void)
{
	while (1);
}

void timer0_isr_handler(void)
{
	unsigned long intoft = INTOFFSET;
	
	SRCPND = 1<<intoft;
	INTPND = 1<<intoft;
	
	isr[intoft]();
}

void timer0_isr(void)
{
	GPBDAT = ~(GPBDAT & (0x1<<5));
}

/*
 *	Timer input clock Frequency = PCLK / {prescaler value+1} / {divider value}
 *	{prescaler value} = 0~255
 *	{divider value} = 2, 4, 8, 16
 */
void timer0_irq_init(void)
{
	int i;

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

	for (i = 0; i < 60; ++i)
		isr[i] = dummy_isr;

	isr[TIMER0_OFT] = timer0_isr; 
}


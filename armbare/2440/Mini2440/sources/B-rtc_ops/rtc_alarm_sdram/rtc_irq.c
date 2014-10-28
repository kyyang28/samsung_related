

#include "s3c2440-regs.h"

void rtc_irq_init(void)
{
	/* Step 1: Interrupt Source - RTC */
	RTCALM = 0x7f;

	/* Step 2: Interrupt Controller */
	INTMSK &= ~(1<<30);
	INTMOD &= ~(1<<30);

	/* Step 3: Enable the I-bit of CPSR */
	__asm__ (
		"mrs r0, cpsr\n"
		"and r0, r0, #~(1<<7)\n"
		"msr cpsr, r0\n"
	);
}

void beep_init_ll(void)
{
	GPBCON |= 0x01; 
}

void beep_isr(void)
{
	GPBDAT |= 0x1;
	DELAY();
	GPBDAT &= ~(0x1);
	DELAY();
}

void rtc_int_handler(void)
{
	unsigned long intoft = INTOFFSET;

	beep_isr();
	
	SRCPND = 1<<intoft;
	INTPND = 1<<intoft;
}


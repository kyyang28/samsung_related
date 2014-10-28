
#include "s3c2440-regs.h"
#include "sd_irq.h"

static void dummyISR(void)
{
	while (1);
}

void sd_int_init(void)
{
	int i;
	
	for (i = 0; i < 60; ++i)
		isr[i] = dummyISR;

	__asm__ (
		"mrs r0, cpsr\n"
		"and r0, r0, #~(1<<7)\n"
		"msr cpsr, r0\n"
	);
}

void sd_isr(void)
{
	u32 oft = INTOFFSET;

	isr[oft]();

	SRCPND = 1<<oft;
	INTPND = 1<<oft;
}


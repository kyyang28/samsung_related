

#include "s3c2440-regs.h"
#include "leds_ops.h"

void key_int_init_ll(void)
{
	/* 1. Setup the interrupt source(keys) */
	GPGCON |= (0x2<<22) | (0x2<<14) | (0x2<<12) | (0x2<<10) | (0x2<<6) | (0x2);
	
	/* Trigger method: Both edge triggered */
	EXTINT1 = (0x7<<28) | (0x7<<24) | (0x7<<20) | (0x7<<12) | (1<<3) | (0x7);
	EXTINT2 = 0x7<<12;
	
	/* External Interrupt Mask Register */
	EINTMASK &= ~(1<<19 | 1<<15 | 1<<14 | 1<<13 | 1<<11 | 1<<8);
}

void key_isr(void)
{
	if (EINTPEND & (1<<8)) {
		if (GPGDAT & (1<<0))
			leds_off(1);
		else 
			leds_on(1);
	}

	if (EINTPEND & (1<<11)) {
		if (GPGDAT & (1<<3))
			leds_off(2);
		else
			leds_on(2);
	}
	
	if (EINTPEND & (1<<13)) {
		if (GPGDAT & (1<<5))
			leds_off(3);
		else
			leds_on(3);
	}

	if (EINTPEND & (1<<14)) {
		if (GPGDAT & (1<<6))
			leds_off(4);
		else
			leds_on(4);
	}

	if (EINTPEND & (1<<15)) {
		if (GPGDAT & (1<<7)) {
			leds_off(1);
			leds_off(3);
		}else {
			leds_on(1);
			leds_on(3);
		}
	}

	if (EINTPEND & (1<<19)) {
		if (GPGDAT & (1<<11)) {
			leds_off(2);
			leds_off(4);
		}else {
			leds_on(2);
			leds_on(4);
		}
	}

	EINTPEND = (1<<8) | (1<<11) | (1<<13) | (1<<14) | (1<<15) | (1<<19);
}


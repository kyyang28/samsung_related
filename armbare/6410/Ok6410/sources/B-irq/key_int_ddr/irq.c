
#include "s3c6410_regs.h"

extern void led_on(int num);
extern void led_off(int num);

void irq_init(void)
{
	int i;

	GPNCON &= ~(0xfff);
	GPNCON |= 0xaaa;

	EINT0CON0 &= ~(0xfff);
	EINT0CON0 |= 0x777;

	for (i = 0; i < 6; ++i)
		EINT0PEND = 1<<i; 	

	EINT0MASK &= ~(0x3f);

	VIC0INTENABLE |= (0x3); /* bit0: eint0~3, bit1: eint4~11 */ 
}

void do_irq(void)
{
	//EINT0PEND   = 0x3f;  /* ÇåÖÐ¶Ï */
	
	if (EINT0PEND & (1<<0)) {
		if (GPNDAT & (1<<0))
			led_off(1);
		else
			led_on(1);
	}else if (EINT0PEND & (1<<1)) {
		if (GPNDAT & (1<<1))
			led_off(2);
		else
			led_on(2);
	}else if (EINT0PEND & (1<<2)) {
		if (GPNDAT & (1<<2)) {
			led_off(3);
		}else {
			led_on(3);
		}
	}else if (EINT0PEND & (1<<3)) {
		if (GPNDAT & (1<<3)) {
			led_off(4);
		}else {
			led_on(4);
		}
	}else if (EINT0PEND & (1<<4)) {
		if (GPNDAT & (1<<4)) {
			led_off(1);
			led_off(2);
		}else {
			led_on(1);
			led_on(2);
		}
	}else if (EINT0PEND & (1<<5)) {
		if (GPNDAT & (1<<5)) {
			led_off(3);
			led_off(4);
		}else {
			led_on(3);
			led_on(4);
		}
	}

	EINT0PEND   = 0x3f;
}


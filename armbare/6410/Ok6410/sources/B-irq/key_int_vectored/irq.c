

#include "s3c6410_regs.h"

extern void led_on(int num);
extern void led_off(int num);

void eint0_3_isr(void)
{
	int i = 0;
		
	while (i < 4) {
		if (EINT0PEND & (1<<i)) {
			if (GPNDAT & (1<<i))
				led_off(i+1);
			else
				led_on(i+1);
		}
		++i;
	}
}


void eint4_11_isr(void)
{
	int i = 4;
	
	while (i < 6) {
		if (EINT0PEND & (1<<i)) {
			if (GPNDAT & (1<<i)) {
				led_off(i-3);	/* led1, led2 */
				led_off(i-1);	/* led3, led4 */
			}else {
				led_on(i-3);
				led_on(i-1);
			}
		}
		++i;
	}
}


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

	VIC0VECTADDR0 = (unsigned int)eint0_3_isr;
	VIC0VECTADDR1 = (unsigned int)eint4_11_isr; 
}

void do_irq(void)
{

	void (*irq_isr)(void);

	irq_isr = (void (*)(void))VIC0ADDRESS;

	irq_isr();

	EINT0PEND   = 0x3f;
	VIC0ADDRESS = 0;
}


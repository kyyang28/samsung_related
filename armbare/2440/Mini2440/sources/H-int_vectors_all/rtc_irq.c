

#include "s3c2440-regs.h"
#include "leds_ops.h"

void rtc_int_init_ll(void)
{
	/* Interrupt Source - RTC */
	RTCALM = 0x7f;
}


void rtc_isr(void)
{
	leds_ops();
}




#include <stdio.h>
#include "s3c2440-regs.h"

void delay(void)
{
	volatile int i;
	for (i = 0; i < 700000; i++);
}

int bcd2dec(char bcd)
{
	int value;
	value = (bcd & 0xf) + (bcd >> 4) * 10;
	return value;
}

char dec2bcd(int dec)
{
	char value;
	value = dec % 10 | (dec / 10) << 4;
	return value;
}

void rtc_setup_time(int year, int month, int date, int hour, int minute, int second)
{
	RTCCON	= RTCCON_RTC_ENABLE;  

	BCDYEAR = dec2bcd(year);
	BCDMON	= dec2bcd(month);
	BCDDATE	= dec2bcd(date);
	BCDHOUR	= dec2bcd(hour);
	BCDMIN	= dec2bcd(minute);
	BCDSEC	= dec2bcd(second);

	RTCCON &= RTCCON_RTC_DISABLE;
}

void rtc_display_time(void)
{
	RTCCON = RTCCON_RTC_ENABLE;
	
	printf( "\r\nThe current RTC time is: %d-%d-%d %d:%d:%d\r\n", 
				bcd2dec(BCDYEAR), bcd2dec(BCDMON), bcd2dec(BCDDATE), 
				bcd2dec(BCDHOUR), bcd2dec(BCDMIN), bcd2dec(BCDSEC) );
	
	RTCCON	&= RTCCON_RTC_DISABLE;

	delay();
}


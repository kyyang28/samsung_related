
#include "stdio.h"
#include "rtc_ops.h"

#define true  1
#define false 0

char *day[8] = {" ", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void rtc_enable(unsigned char bdata)
{
	unsigned long uread;
	uread = RTCCON;
	RTCCON = (uread & ~(1<<0)) | (bdata);
}

void rtc_ticktime_enable(unsigned char bdata)
{
	unsigned long uread;
	uread = RTCCON;
	RTCCON = ( (uread & ~(1<<8)) | (bdata<<8) );
}

void rtc_print(void)
{
	unsigned long uyear, umonth, udate, uday, uhour, umin, usec;

	uyear = BCDYEAR;
	uyear = 0x2000 + uyear;
	umonth= BCDMON;
	udate = BCDDAYWEEK;
	uhour = BCDHOUR;
	umin  = BCDMIN;
	usec  = BCDSEC;
	uday  = BCDDAY;
	
	printf("%2x : %2x : %2x  %10s,  %2x/%2x/%4x\r\n", uhour, umin, usec, day[uday], umonth, udate, uyear);
}

void rtc_settime(void)
{
	unsigned long year = 12;
	unsigned long month = 5;
	unsigned long date = 1;
	unsigned long hour = 12;
	unsigned long min = 0;
	unsigned long sec = 0;
	unsigned long weekday= 3;

	year	= (((year/100)<<8) + (((year/10)%10)<<4) + (year%10));
	month	= ( ((month/10)<<4)+ (month%10) );
	date	= ( ((date/10)<<4) + (date%10) );
	weekday = (weekday%10);							
	hour	= ( ((hour/10)<<4) + (hour%10) );
	min		= ( ((min/10)<<4)  + (min%10) );
	sec		= ( ((sec/10)<<4)  + (sec%10) );

	rtc_enable(true);

	BCDSEC  = sec;
	BCDMIN  = min;
	BCDHOUR = hour;
	BCDDAYWEEK = date;
	BCDDAY  = weekday;
	BCDMON  = month;
	BCDYEAR = year;

	rtc_enable(false);
	
	printf("reset success\r\n");
}

void rtc_realtime_display(void)
{
	int counter = 0;
	unsigned long usec = 0;

	rtc_enable(true);

	rtc_ticktime_enable(true);

	while( counter < 5) {
		if(usec != BCDSEC) {
			usec  = BCDSEC;
			rtc_print();
			counter++;
		}
	}

	rtc_ticktime_enable(false);

	rtc_enable(false);
}


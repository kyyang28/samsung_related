

#define RTCCON					(*(volatile unsigned char *)0x57000040)
#define TICNT					(*(volatile unsigned char *)0x57000044)
#define RTCALM					(*(volatile unsigned char *)0x57000050)
#define ALMSEC					(*(volatile unsigned char *)0x57000054)
#define ALMMIN					(*(volatile unsigned char *)0x57000058)
#define ALMHOUR					(*(volatile unsigned char *)0x5700005C)
#define ALMDATE					(*(volatile unsigned char *)0x57000060)
#define ALMMON					(*(volatile unsigned char *)0x57000064)
#define ALMYEAR					(*(volatile unsigned char *)0x57000068)
#define BCDSEC					(*(volatile unsigned char *)0x57000070)
#define BCDMIN					(*(volatile unsigned char *)0x57000074)
#define BCDHOUR					(*(volatile unsigned char *)0x57000078)
#define BCDDATE					(*(volatile unsigned char *)0x5700007C)
#define BCDDAY					(*(volatile unsigned char *)0x57000080)
#define BCDMON					(*(volatile unsigned char *)0x57000084)
#define BCDYEAR					(*(volatile unsigned char *)0x57000088)

#define RTCCON_RTC_ENABLE		(1)
#define RTCCON_RTC_DISABLE		(0)

extern void serial_puts_ll(char *str);
extern int serial_printf(const char * format, ...);

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
	
	serial_printf( "\r\nThe current RTC time is: %d-%d-%d %d:%d:%d\r\n", 
				bcd2dec(BCDYEAR), bcd2dec(BCDMON), bcd2dec(BCDDATE), 
				bcd2dec(BCDHOUR), bcd2dec(BCDMIN), bcd2dec(BCDSEC) );
	
	RTCCON	&= RTCCON_RTC_DISABLE;

	delay();
}


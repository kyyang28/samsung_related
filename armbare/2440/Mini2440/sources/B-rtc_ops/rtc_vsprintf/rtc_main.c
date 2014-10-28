
#include <stdio.h>
#include "s3c2440-regs.h"

extern void uart_init_ll(void);
extern char awaitkey(unsigned long dly);
extern void rtc_setup_time(int year, int month, int date, int hour, int minute, int second);
extern void rtc_display_time(void);

int rtc_main(void)
{
	int year, month, date, hour, minute, second;

	/* Uart initialization */
	uart_init_ll();	
		
	printf("\r\n");
	printf("\r\n+------------------------+");
	printf("\r\n|   RTC Setup Routine!   |");
	printf("\r\n+------------------------+");
	printf("\r\n");

	printf("\r\nPlease enter the year, month, date, hour, minute, and second: \r\n");
	//scanf("%x %x %x %x %x %x %x", &year, &month, &date, &weekday, &hour, &minute, &second);
	scanf("%d %d %d %d %d %d", &year, &month, &date, &hour, &minute, &second);

	/* Setup the rtc time */
	rtc_setup_time(year, month, date, hour, minute, second);

	printf("\r\n");
	printf("\r\n+--------------------------+");
	printf("\r\n|   RTC setup completed!   |");
	printf("\r\n+--------------------------+");
	printf("\r\n");

	while (1) {
		/* RTC display routine */
		rtc_display_time();
	}

	return 0;
}


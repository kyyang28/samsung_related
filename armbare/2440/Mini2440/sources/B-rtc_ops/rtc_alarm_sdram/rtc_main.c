
#include <stdio.h>
#include "s3c2440-regs.h"

extern void beep_init_ll(void);
extern void beep_isr(void);
extern void rtc_irq_init(void);
extern void uart_init_ll(void);
extern void rtc_setup_time(int year, int month, int date, int hour, int minute, int second);
extern void rtc_alarm_setup_time(int year, int month, int date, int hour, int minute, int second);
extern void rtc_display_time(void);

int rtc_main(void)
{
	int year, month, date, hour, minute, second;

	/* Beep initialization */
	beep_init_ll();
	
	/* RTC irq initialization */
	rtc_irq_init();
	
	/* Uart initialization */
	uart_init_ll();	

	printf("\r\n");
	printf("\r\n+------------------------+");
	printf("\r\n|   RTC Setup Routine!   |");
	printf("\r\n+------------------------+");
	printf("\r\n");

	printf("\r\nPlease enter the year, month, date, hour, minute, and second: \r\n");
	scanf("%d %d %d %d %d %d", &year, &month, &date, &hour, &minute, &second);

	/* Setup the rtc time */
	rtc_setup_time(year, month, date, hour, minute, second);

	printf("\r\n");
	printf("\r\n+--------------------------+");
	printf("\r\n|   RTC setup completed!   |");
	printf("\r\n+--------------------------+");
	printf("\r\n");


	printf("\r\n");
	printf("\r\n+------------------------------+");
	printf("\r\n|   RTC Alarm Setup Routine!   |");
	printf("\r\n+------------------------------+");
	printf("\r\n");

	printf("\r\nPlease enter the year, month, date, hour, minute, and second: \r\n");
	scanf("%d %d %d %d %d %d", &year, &month, &date, &hour, &minute, &second);

	/* Setup the rtc time */
	rtc_alarm_setup_time(year, month, date, hour, minute, second);

	printf("\r\n");
	printf("\r\n+--------------------------------+");
	printf("\r\n|   RTC Alarm Setup Completed!   |");
	printf("\r\n+--------------------------------+");
	printf("\r\n");

	while (1) {
		/* RTC display routine */
		rtc_display_time();
	}

	return 0;
}


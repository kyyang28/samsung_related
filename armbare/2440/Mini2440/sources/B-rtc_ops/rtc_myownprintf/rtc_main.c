
extern void uart_init_ll(void);
extern void rtc_display_time(void);
extern int serial_printf(const char * format, ...);
extern void rtc_setup_time(int year, int month, int date, int hour, int minute, int second);

int rtc_main(void)
{
	int year, month, date, hour, minute, second;

	/* Uart initialization */
	uart_init_ll();	
		
	serial_printf("\r\n");
	serial_printf("\r\n+------------------------+");
	serial_printf("\r\n|   RTC Setup Routine!   |");
	serial_printf("\r\n+------------------------+");
	serial_printf("\r\n");

	year	= 12;
	month	= 12;
	date	= 10;
	hour	= 22;
	minute	= 40;
	second	= 28;

	serial_printf("\r\nyear = %d, month = %d, date = %d, hour = %d, minute = %d, second = %d", 
				   year, month, date, hour, minute, second);

	/* Setup the rtc time */
	rtc_setup_time(year, month, date, hour, minute, second);

	serial_printf("\r\n");
	serial_printf("\r\n+--------------------------+");
	serial_printf("\r\n|   RTC setup completed!   |");
	serial_printf("\r\n+--------------------------+");
	serial_printf("\r\n");

	while (1) {
		/* RTC display routine */
		rtc_display_time();
	}

	return 0;
}


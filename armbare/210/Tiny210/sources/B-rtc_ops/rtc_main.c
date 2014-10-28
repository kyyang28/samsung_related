
#include "stdio.h"
#include "uart_ops.h"
#include "rtc_ops.h"

int rtc_main(void)
{
	int ch = 0;
	
	/* Uart init */
	uart_init_ll();

	while (1) {
		printf("\r\n############### RTC Testing Program ##############\r\n");
		printf("[d] Display rtc realtime (hour:min:sec:weekday date/month/year)\r\n");
		printf("[s] Reset rtc realtime(12:0:0:Tuesday 1/1/2012) \r\n");
		printf("Enter your choice:");
		
		ch = uart_getc();
		printf("%c\r\n",ch);

		switch (ch) {
			case 'd':
				rtc_realtime_display();
				break;

			case 's':
				rtc_settime();
				break;
		}
	}

	return 0;
}


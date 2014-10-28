#ifndef __RTC_OPS_H
#define __RTC_OPS_H

void rtc_setup_time(int year, int month, int date, int hour, int minute, int second);
void rtc_alarm_setup_time(int year, int month, int date, int hour, int minute, int second);
void rtc_display_time(void);

#endif


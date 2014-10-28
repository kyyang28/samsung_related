#ifndef __RTC_OPS_H
#define __RTC_OPS_H

#define INTP								(*(volatile unsigned long *)0xE2800030)
#define RTCCON								(*(volatile unsigned long *)0xE2800040)
#define TICCNT								(*(volatile unsigned long *)0xE2800044)
#define RTCALM								(*(volatile unsigned long *)0xE2800050)
#define ALMSEC								(*(volatile unsigned long *)0xE2800054)
#define ALMMIN								(*(volatile unsigned long *)0xE2800058)
#define ALMHOUR								(*(volatile unsigned long *)0xE280005C)
#define ALMDAY								(*(volatile unsigned long *)0xE2800060)
#define ALMMON								(*(volatile unsigned long *)0xE2800064)
#define ALMYEAR								(*(volatile unsigned long *)0xE2800068)
#define BCDSEC								(*(volatile unsigned long *)0xE2800070)
#define BCDMIN								(*(volatile unsigned long *)0xE2800074)
#define BCDHOUR								(*(volatile unsigned long *)0xE2800078)
#define BCDDAYWEEK							(*(volatile unsigned long *)0xE280007C)
#define BCDDAY								(*(volatile unsigned long *)0xE2800080)
#define BCDMON								(*(volatile unsigned long *)0xE2800084)
#define BCDYEAR								(*(volatile unsigned long *)0xE2800088)
#define CURTICCNT							(*(volatile unsigned long *)0xE2800090)

void rtc_settime(void);
void rtc_realtime_display(void);

#endif


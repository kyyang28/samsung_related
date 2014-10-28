#ifndef __TIMER_OPS_H
#define __TIMER_OPS_H

#define TCFG0								(*(volatile unsigned long *)0xE2500000)
#define TCFG1								(*(volatile unsigned long *)0xE2500004)
#define TCON								(*(volatile unsigned long *)0xE2500008)
#define TCNTB0								(*(volatile unsigned long *)0xE250000C)
#define TCMPB0								(*(volatile unsigned long *)0xE2500010)
#define TCNTO0								(*(volatile unsigned long *)0xE2500014)
#define TCNTB1								(*(volatile unsigned long *)0xE2500018)
#define TCMPB1								(*(volatile unsigned long *)0xE250001C)
#define TCNTO1								(*(volatile unsigned long *)0xE2500020)
#define TCNTB2								(*(volatile unsigned long *)0xE2500024)
#define TCMPB2								(*(volatile unsigned long *)0xE2500028)
#define TCNTO2								(*(volatile unsigned long *)0xE250002C)
#define TCNTB3								(*(volatile unsigned long *)0xE2500030)
#define TCMPB3								(*(volatile unsigned long *)0xE2500034)
#define TCNTO3								(*(volatile unsigned long *)0xE2500038)
#define TCNTB4								(*(volatile unsigned long *)0xE250003C)
#define TCNTO4								(*(volatile unsigned long *)0xE2500040)
#define TINT_CSTAT							(*(volatile unsigned long *)0xE2500044)

int timer0_init_ll(void);
void timer0_irq_handler(void);

#endif


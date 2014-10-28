
#include <stdio.h>
#include "../include/mini2440_regs.h"

extern int tstc(void);
extern unsigned char serial_getc(void);
extern void displayHWTestMenuUsage(void);

void led_init(void)
{
    GPBCON |= (1<<10) | (1<<12) | (1<<14) | (1<<16);
}

void led_on(int num)
{
    switch (num) {
        case 1:
        case 2:
        case 3:
        case 4:
            GPBDAT = (~(1<<(num+4)));
            break;
        default:
            printf("\r\nInvalid LED number, please try 1,2,3,4!\r\n");
            break;
    }
}

void led_off(int num)
{
    switch (num) {
        case 1:
        case 2:
        case 3:
        case 4:
            GPBDAT = (1<<(num+4));
            break;
        default:
            printf("\r\nInvalid LED number, please try 1,2,3,4!\r\n");
            break;
    }
}

void led_all_on(void)
{
    GPBDAT = ~(1<<5 | 1<<6 | 1<<7 | 1<<8);
}

void led_all_off(void)
{
    GPBDAT = (1<<5 | 1<<6 | 1<<7 | 1<<8);
}


inline void DELAY (unsigned long ticks)
{
	__asm__ volatile (
	  "1:\n"
	  "subs %0, %1, #1\n"
	  "bne 1b":"=r" (ticks):"0" (ticks));
}

void ledRun(void)
{
    volatile unsigned long i;
    
	while(1) {
		DELAY(0x1200000);
		GPBDAT = (~(i<<5));
		if(++i == 16)
			i = 0;

        if (tstc()) {
            if ('z' == serial_getc() || 'Z' == serial_getc()) {
                led_all_off();
                displayHWTestMenuUsage();
                break;
            }
        }
	}
}

void led_test(void)
{
    led_init();
    led_all_off();
    //ledRun();
    led_on(1);
}


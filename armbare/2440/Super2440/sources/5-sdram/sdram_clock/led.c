/*
 *	Descriptions: S3C2440 Board, LED driver program
 *
 *	Author:	Pete & Charles
 *
 *	Date: March 3rd, 2012
 *
*/

#include "s3c2440.h"

/*
#define DELAY(times)    do {    \
                            int i, j;   \
                            for(i = 0; i < times; ++i) \
                            for(j = 0; j < 0x10000; ++j); \
                        }while(0)   \
*/

static inline void DELAY (unsigned long ticks)
{
	__asm__ volatile (
	  "1:\n"
	  "subs %0, %1, #1\n"
	  "bne 1b":"=r" (ticks):"0" (ticks));
}

void led_on(int num)
{
    if(num == 1)
        GPFDAT &= ~(1<<(num+5));
    else if(num == 2)
        GPGDAT &= ~(1<<(num-2));
    else if(num == 3)
        GPGDAT &= ~(1<<(num-2));
    else if(num == 4)
        GPGDAT &= ~(1<<(num+6));
    else {
        GPFDAT |= 1<<6;
        GPGDAT |= (1<<0 | 1<<1 | 1<<10);
    }   
}


void led_off(int num)
{
    if(num == 1)
        GPFDAT |= 1<<(num+5);
    else if(num == 2)
        GPGDAT |= 1<<(num-2);
    else if(num == 3)
        GPGDAT |= 1<<(num-2);
    else if(num == 4)
        GPGDAT |= 1<<(num+6);
    else {
        GPFDAT |= 1<<6;
        GPGDAT |= (1<<0 | 1<<1 | 1<<10);
    }
}


void led_all_off(void)
{
    GPFDAT |= 1<<6;
    GPGDAT |= (1<<0 | 1<<1 | 1<<10);
}


void led_all_on(void)
{
    GPFDAT &= ~(1<<6);
    GPGDAT &= ~(1<<0 | 1<<1 | 1<<10);
}


void LedRun(void)
{
    led_all_off();
    while(1) 
    {
        led_on(1);
        DELAY(200000);
        led_off(1);      
        
        led_on(2);
        DELAY(200000);
        led_off(2);
        
        led_on(3);
        DELAY(200000);          
        led_off(3);
                          
        led_on(4);         
        DELAY(200000);          
        led_off(4);              
    }
}


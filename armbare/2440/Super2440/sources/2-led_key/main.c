/*
 *	Descriptions: S3C2440 Board, LED driver program
 *
 *	Author:	Pete & Charles
 *
 *	Date: March 3rd, 2012
 *
*/

#include "key.h"


#define DELAY(times)    do {    \
                            int i, j;   \
                            for(i = 0; i < times; ++i) \
                            for(j = 0; j < 0x10000; ++j); \
                        }while(0)   \


void led_init(void)
{
    // GPF6 LED1  [13:12]   01 = Output
    GPFCON |= 1<<12;
    GPFCON &= ~(1<<13);
                            
    // GPG0 LED2    [1:0]       01 = Output
    GPGCON |= 1<<0;
    GPGCON &= ~(1<<1);
                                            
    // GPG1 LED3    [3:2]       01 = Output
    GPGCON |= 1<<2;
    GPGCON &= ~(1<<3);
                                                            
    // GPG10 LED4   [21:20]     01 = Output
    GPGCON |= 1<<20;
    GPGCON &= ~(1<<21);
                                                                            
    // µÆÈ«Ãð
    GPFDAT |= 1<<6;
    GPGDAT |= (1<<0 | 1<<1 | 1<<10);
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


int mymain(void)
{
		
	led_init();
	key_init();

	while(1) 
	{
		if(GPFDAT & (1<<1))
			led_off(1);
		else
			led_on(1);

		if(GPFDAT & (1<<3))
			led_off(2);
		else
			led_on(2);

		if(GPFDAT & (1<<5))
			led_off(3);
		else
			led_on(3);

		if(GPFDAT & (1<<0))
			led_off(4);
		else
			led_on(4);
	}

	return 0;	
}


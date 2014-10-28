
#include "s3c2440.h"

void disable_watchdog(void)
{
	WTCON = 0;	
}


void clock_init(void) 
{
	S3C2440_CLOCK * const clock = S3C2440_Get_CLOCK_Base();

	//locktime
	clock->LOCKTIME = 0x00FFFFFF;
	
	//clock divide
	clock->CLKDIVN |= (1<<0 | 1<<2);
	clock->CLKDIVN &= ~(1<<1);

	__asm__ (
		"mrc  p15, 0, r1, c1, c0, 0\n" 
		"orr  r1, r1, #0xC0000000\n" 
		"mcr  p15, 0, r1, c1, c0, 0\n"
	);

	clock->MPLLCON = (MDIV << 12) | (PDIV << 4) | (SDIV << 0);
}


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
    
    // light all off
    GPFDAT |= 1<<6;
    GPGDAT |= (1<<0 | 1<<1 | 1<<10);
}


void key_init(void)
{
	/*
	 *	GPFCON  0x56000050  R/W  Configures the pins of port F 
	 *	
	 *	GPF1  [3:2]  		00 = Input           01 = Output 
	 *				10 = EINT[1]         11 = Reserved
	 *
	 *	Key 2 
	 */
	GPFCON &= ~(1<<2 | 1<<3);

	/*
	 *	GPFCON  0x56000050  R/W  Configures the pins of port F 
	 *	
	 *	GPF3  [7:6]  	00 = Input               01 = Output 
	 *			10 = EINT[3]             11 = Reserved
	 *
	 *	Key 3 
	 */
	GPFCON &= ~(1<<6 | 1<<7);

	
	/*
	 *	GPFCON  0x56000050  R/W  Configures the pins of port F 
	 *
	 *	GPF5  [11:10]  	00 = Input               01 = Output 
	 *			10 = EINT[5]             11 = Reserved 
	 *
	 *	Key 4 
	 */
	GPFCON &= ~(1<<10 | 1<<11);


	/*
	 *	GPFCON  0x56000050  R/W  Configures the pins of port F 
	 *
	 *	GPF0  [1:0]  	00 = Input               01 = Output 
	 *			10 = EINT[0]             11 = Reserved 
	 *
	 *	Key 5 
	 */
	GPFCON &= ~(1<<0 | 1<<1);


	/*
	 *	GPFCON  0x56000050  R/W  Configures the pins of port F 
	 *
	 *	GPF2  [5:4]  	00 = Input               01 = Output 
	 *			10 = EINT[2]             11 = Reserved 
	 *
	 *	Key 6 
	 */
	GPFCON &= ~(1<<4 | 1<<5);


	/*
	 *	GPFCON  0x56000050  R/W  Configures the pins of port F 
	 *
	 *	GPF4  [9:8]  	00 = Input               01 = Output 
	 *			10 = EINT[4]             11 = Reserved
	 *
	 *	Key 7 
	 */
	GPFCON &= ~(1<<8 | 1<<9);
}


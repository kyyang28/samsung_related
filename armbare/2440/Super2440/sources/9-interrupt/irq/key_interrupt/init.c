
#include "s3c2440.h"


void disable_watchdog(void)
{
	WTCON = 0;
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


void key_interrupt_init(void)
{
	/*
	 *	GPFCON  0x56000050  R/W  Configures the pins of port F 
	 *	
	 *	GPF1  [3:2]  		00 = Input           01 = Output 
	 *				10 = EINT[1]         11 = Reserved
	 *
	 *	Key 2 
	 */
	GPFCON &= ~(1<<2); 
	GPFCON |= 1<<3;

	/*
	 *	GPFCON  0x56000050  R/W  Configures the pins of port F 
	 *	
	 *	GPF3  [7:6]  	00 = Input               01 = Output 
	 *			10 = EINT[3]             11 = Reserved
	 *
	 *	Key 3 
	 */
	GPFCON |= 1<<7; 
	GPFCON &= ~(1<<6);

	
	/*
	 *	GPFCON  0x56000050  R/W  Configures the pins of port F 
	 *
	 *	GPF5  [11:10]  	00 = Input               01 = Output 
	 *			10 = EINT[5]             11 = Reserved 
	 *
	 *	Key 4 
	 */
	GPFCON |= 1<<11;
	GPFCON &= ~(1<<10);


	/*
	 *	GPFCON  0x56000050  R/W  Configures the pins of port F 
	 *
	 *	GPF0  [1:0]  	00 = Input               01 = Output 
	 *			10 = EINT[0]             11 = Reserved 
	 *
	 *	Key 5 
	 */
	GPFCON |= 1<<1;
	GPFCON &= ~(1<<0);

	// GPF2	-- EINT2
	GPFCON |= 1<<5;
	GPFCON &= ~(1<<4);
	
	// GPF4 -- EINT4
	GPFCON |= 1<<9;
	GPFCON &= ~(1<<8);
}


void irq_init(void)
{
	  /* Interrupt Source */
		key_interrupt_init();
		
		/* 
		 * External Interrupt Configuration *
		 * Only need to enable EINT4 and EINT5, since [3:0] Reserved
		 */
		EINTMASK &= ~(1<<4 | 1<<5);
		
		/* Interrupt Mode -- IRQ or FIQ */
		INTMOD &= ~(1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4);	// Choose IRQ
		
		/* Interrupt Mask -- Interrupt service is available or not */
		INTMSK &= ~(1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4);
		
		/* PRIORITY configuration */
		//PRIORITY 
}


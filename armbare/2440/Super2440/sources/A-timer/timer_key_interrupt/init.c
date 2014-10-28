
#include "s3c2440.h"


void led_init(void)
{
    // GPF6 LED1  [13:12]       01 = Output
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


void timer0_init(void)
{
    TCFG0 = 99;    /* Prescaler */
    TCFG1 = 0x3;   /* MUX : 0011 = 1/16 */
    TCNTB0 = 15625; // 0.5s
    // The initial value of TCMPB0 is 0, so we don't need to setup manually.
    
    /* 1<<0 (Timer0 start/stop), Timer0 manual update, Timer0 auto reload on/off */
    TCON |= (1<<1);
    TCON = 0x09; // TCON = 0x0D is fine(Timer0 output invertor on/off)
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
    
    INTMOD &= ~(1<<10);
    INTMSK &= ~(1<<10);

    __asm__ (
        "msr cpsr_c, #0x5F\n"
    );
}


void copy_from_SRAM_to_SDRAM(void)
{
    unsigned int *SRAM_ADDR = (unsigned int *)0;
    unsigned int *SDRAM_ADDR = (unsigned int *)0x30000000;

    for( ; SRAM_ADDR < (unsigned int *)4096; SRAM_ADDR++, SDRAM_ADDR++)
        *SDRAM_ADDR = *SRAM_ADDR;
}



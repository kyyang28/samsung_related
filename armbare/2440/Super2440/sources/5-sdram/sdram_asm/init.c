
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


void copy_from_SRAM_to_SDRAM(void)
{
	unsigned int *SRAM_ADDR = (unsigned int *)0;
	unsigned int *SDRAM_ADDR = (unsigned int *)0x30000000;

	for( ; SRAM_ADDR < (unsigned int *)4096; SRAM_ADDR++, SDRAM_ADDR++)
		*SDRAM_ADDR = *SRAM_ADDR;
}



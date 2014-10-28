
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


void sdram_init(void)
{
	S3C2440_SDRAM * const sdram = S3C2440_Get_SDRAM_Base();
	
	// BWSCON configuration Bank Bus Width
	sdram->BWSCON = (B7_BWSCON<<28) | (B6_BWSCON<<24) | (B5_BWSCON<<20) | \
			 (B4_BWSCON<<16) | (B3_BWSCON<<12) | (B2_BWSCON<<8)  | \
			 (B1_BWSCON<<4);

	// BANKCON0 configuration
	sdram->BANKCON0 = (B0_Tacs<<13) | (B0_Tcos<<11) | (B0_Tacc<<8) | (B0_Tcoh<<6) | \
			   (B0_Tcah<<4) | (B0_Tacp<<2) | (B0_PMC<<0); 

	// BANKCON1 configuration
	sdram->BANKCON1 = (B1_Tacs<<13) | (B1_Tcos<<11) | (B1_Tacc<<8) | (B1_Tcoh<<6) | \
			   (B1_Tcah<<4) | (B1_Tacp<<2) | (B1_PMC<<0); 

	// BANKCON2 configuration
	sdram->BANKCON2 = (B2_Tacs<<13) | (B2_Tcos<<11) | (B2_Tacc<<8) | (B2_Tcoh<<6) | \
			   (B2_Tcah<<4) | (B2_Tacp<<2) | (B2_PMC<<0); 

	// BANKCON3 configuration
	sdram->BANKCON3 = (B3_Tacs<<13) | (B3_Tcos<<11) | (B3_Tacc<<8) | (B3_Tcoh<<6) | \
			   (B3_Tcah<<4) | (B3_Tacp<<2) | (B3_PMC<<0); 

	// BANKCON4 configuration
	sdram->BANKCON4 = (B4_Tacs<<13) | (B4_Tcos<<11) | (B4_Tacc<<8) | (B4_Tcoh<<6) | \
			   (B4_Tcah<<4) | (B4_Tacp<<2) | (B4_PMC<<0); 

	// BANKCON5 configuration
	sdram->BANKCON5 = (B5_Tacs<<13) | (B5_Tcos<<11) | (B5_Tacc<<8) | (B5_Tcoh<<6) | \
			   (B5_Tcah<<4) | (B5_Tacp<<2) | (B5_PMC<<0); 

	// BANKCON6 configuration
	sdram->BANKCON6 = (B6_MT<<15) | (B6_Trcd<<2) | (B6_SCAN<<0);
	
	// BANKCON7 configuration
	sdram->BANKCON7 = (B7_MT<<15) | (B7_Trcd<<2) | (B7_SCAN<<0);

	// REFRESH configuration
	sdram->REFRESH = (REFEN<<23) | (TREFMD<<22) | (Trp<<20) | (Tsrc<<18) | \
	                  (REFCNT<<0);

	// BANKSIZE configuration
	sdram->BANKSIZE = (BURST_EN<<7) | (SCKE_EN<<5) | (SCLK_EN<<4) | (BK76MAP<<0); 
	
	// MRSRB6 configuration
	sdram->MRSRB6 = (CL<<4);
	
	// MRSRB7 configuration
	sdram->MRSRB7 = (CL<<4);
}


void copy_from_SRAM_to_SDRAM(void)
{
	unsigned int *SRAM_ADDR = (unsigned int *)0;
	unsigned int *SDRAM_ADDR = (unsigned int *)0x30000000;

	for( ; SRAM_ADDR < (unsigned int *)4096; SRAM_ADDR++, SDRAM_ADDR++)
		*SDRAM_ADDR = *SRAM_ADDR;
}



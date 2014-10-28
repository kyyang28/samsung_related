
#include "s3c2440.h"


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




#include "s3c2440-regs.h"

void delay(int tick)
{
	u32 val = (PCLK>>3) / 1000 - 1;
			
	TCFG0 &= ~(0xff<<8);
	TCFG0 |= 3<<8;			//prescaler = 3+1
	TCFG1 &= ~(0xf<<12);
	TCFG1 |= 0<<12;		//mux = 1/2

	TCNTB3 = val;
	TCMPB3 = val>>1;		// 50%
	TCON &= ~(0xf<<16);
	TCON |= 0xb<<16;		//interval, inv-off, update TCNTB3&TCMPB3, start timer 3
	TCON &= ~(2<<16);		//clear manual update bit
	
	while(tick--) {
		while(TCNTO3 >= val>>1);
		while(TCNTO3 < val>>1);
	};
}

/* Make compilation w/o error */
int raise (int signum)
{
	    return 0;
}


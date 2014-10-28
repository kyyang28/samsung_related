
#include "s3c6410_regs.h"

void ddr_init(void)
{
	unsigned int tRCD = NS2CLOCK(23);
	unsigned int tRFC = NS2CLOCK(80);
	unsigned int tRP  = NS2CLOCK(23);

	/* DDR configure */
	P1MEMCCMD = 0x4;

	/* Setup refresh period */
	P1REFRESH = NS2CLOCK(7800);

	/* Setup DDR timing parameters */
	P1CASLAT = 0x3<<1;
	P1T_DQSS = 0x1;
	P1T_MRD  = 0x2;
	P1T_RAS  = NS2CLOCK(45);
	P1T_RC   = NS2CLOCK(68);
	P1T_RCD  = (tRCD | ((tRCD - 3)<<3));
	P1T_RFC  = (tRFC | ((tRFC - 3)<<5));
	P1T_RP	 = (tRP | ((tRP - 3)<<3));
	P1T_RRD  = NS2CLOCK(15);
	P1T_WR	 = NS2CLOCK(15);
	P1T_WTR  = 0x7;
	P1T_XP   = 0x2;
	P1T_XSR  = NS2CLOCK(120);
	P1T_ESR  = NS2CLOCK(120);

	/* Setup DDR Mem Config */
	P1MEMCFG &= ~(0x7);
	P1MEMCFG |= 0x2;		/* column address 10-bit */
	P1MEMCFG &= ~(0x7<<3);
	P1MEMCFG |= 0x3<<3;		/* Row address 14-bit */
	P1MEMCFG &= ~(1<<6);
	P1MEMCFG &= ~(0x7<<15);
	P1MEMCFG |= 0x2<<15;

	P1MEMCFG2 &= ~(0xF);
	P1MEMCFG2 |= 0x5;
	P1MEMCFG2 &= ~(0x3<<6);
	P1MEMCFG2 |= 0x1<<6;
	P1MEMCFG2 &= ~(0x7<<8);
	P1MEMCFG2 |= 0x3<<8;	/* Mobile DDR SDRAM */
	P1MEMCFG2 &= ~(0x3<<11);
	P1MEMCFG2 |= 0x1<<11;	/* Read delay 1 cycle(usually for mobile DDR SDRAM) */

	P1_chip_0_cfg |= 1<<16;	/* Bank-Row-Column organization */

	/* Mobile DDR SDRAM initialization sequence */
	P1DIRECTCMD = 0xC0000;	/* NOP */
	P1DIRECTCMD = 0x000;	/* PrechargeAll */
	P1DIRECTCMD = 0x40000;	/* Autorefresh */
	P1DIRECTCMD = 0x40000;	/* Autorefresh */
	P1DIRECTCMD = 0x80032;	/* MRS */
	P1DIRECTCMD = 0xA0000;	/* EMRS */

	MEM_SYS_CFG = 0x0;

	/* Setup DDR status to "GO" */
	P1MEMCCMD = 0x000;
	
	/* Wait for DDR to be ready */
	while (!((P1MEMSTAT & 0x3) == 0x1));
}



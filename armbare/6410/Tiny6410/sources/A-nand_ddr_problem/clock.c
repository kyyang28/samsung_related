
#include "s3c6410_regs.h"

void checkSYNCACK(void)
{
	while ((OTHERS & 0xF00) != 0);
}

void clock_init(void)
{
//#define EPLL0_VAL   ( (2<<0)|(1<<8)|(32<<16)|PLL_ENABLE)
//#define EPLL1_VAL	( 0 )

	/* Setup the PLL lock time */
	APLL_LOCK = 0xFFFF;
	MPLL_LOCK = 0xFFFF;
	EPLL_LOCK = 0xFFFF;
	
	/* 
	 *	SYNCMODE   [7]  SYNCMODEREQ to ARM
	 *					0: Asynchronous mode
	 *	SYNCMUXSEL [6]  SYS CLOCK SELECT IN CMU
	 *					0: MOUT_MPLL
	 */
	OTHERS &= ~(1<<7 | 1<<6);
	/* Wait until the SYNC returns an ACK */
	checkSYNCACK();
	
	/* Setup the divider */
	CLK_DIV0 = (PCLK_RATIO<<12 | HCLKX2_RATIO<<9 | HCLK_RATIO<<8 | MPLL_RATIO<<4 | ARM_RATIO);	
	
	/* Setup the PLL value (533MHz) */
	APLL_CON = APLLCON_VAL;
	MPLL_CON = MPLLCON_VAL;
	//EPLL_CON0 = EPLL0_VAL;
	//EPLL_CON1 = EPLL1_VAL;
	
	/* 
	 *	MPLL_SEL [1] Control MUXMPLL	1:FOUT_MPLL
	 *	APLL_SEL [0] Control MUXAPLL	1:FOUT_APLL
	 */
	CLK_SRC = 0x3;
}



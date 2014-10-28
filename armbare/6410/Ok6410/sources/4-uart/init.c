
#include "s3c6410_regs.h"

void checkSYNCACK(void)
{
	while ((OTHERS & 0xF00) != 0);
}

void clock_init(void)
{
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
	
	/* 
	 *	MPLL_SEL [1] Control MUXMPLL	1:FOUT_MPLL
	 *	APLL_SEL [0] Control MUXAPLL	1:FOUT_APLL
	 */
	CLK_SRC = 0x3;
}

void uart_init(void)
{
	/* Setup the GPIO for RXD and TXD */
	GPACON &= ~(0xFF);
	GPACON |= 0x22;
	
	/* The number of data bits = 11(8-bit) */
	ULCON0 = 0x3;
	
	/* 
	 *	Clock Selection [11:10] 10 = PCLK : DIV_VAL = (PCLK / (bps x 16) ) - 1 
	 *	Transmit/Receive Modes: 01 = Interrupt request or polling mode 	
	 */
	UCON0 = 0x805;
	
	/* FIFO Enable: 1 = Enable */
#ifdef ENABLE_UART_FIFO
	UFCON0 = 0x1;
#endif

	/* Baudrate divisor */
	UBRDIV0 = 35;	/* PCLK = 67MHz, Baudrate = 115200 */
	
	/* The number of 1's in UBRDIV0 */
	UDIVSLOT0 = 0x0080;	/* The number of 1's is 1 */
}



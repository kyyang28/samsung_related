#ifndef __S3C6410_REGS_H__
#define __S3C6410_REGS_H__

/* LED */
#define GPKCON0						(*(volatile unsigned long *)0x7F008800)
#define GPKDAT						(*(volatile unsigned long *)0x7F008808)

/* CLOCK */
#define APLL_LOCK					(*(volatile unsigned long *)0x7E00F000)
#define MPLL_LOCK					(*(volatile unsigned long *)0x7E00F004)
#define EPLL_LOCK					(*(volatile unsigned long *)0x7E00F008)
#define APLL_CON					(*(volatile unsigned long *)0x7E00F00C)
#define MPLL_CON					(*(volatile unsigned long *)0x7E00F010)
#define EPLL_CON0					(*(volatile unsigned long *)0x7E00F014)
#define EPLL_CON1					(*(volatile unsigned long *)0x7E00F018)
#define CLK_SRC						(*(volatile unsigned long *)0x7E00F01C)
#define	CLK_DIV0					(*(volatile unsigned long *)0x7E00F020)
#define OTHERS						(*(volatile unsigned long *)0x7E00F900)

/* 533MHz */
#define	MDIV						(266)
#define	PDIV						(3)
#define	APLL_SDIV					(1)
#define	MPLL_SDIV					(2)
#define	PLL_ENABLE					(1<<31)
#define	APLLCON_VAL					(PLL_ENABLE | ((MDIV)<<16) | ((PDIV)<<8) | (APLL_SDIV))
#define	MPLLCON_VAL					(PLL_ENABLE | ((MDIV)<<16) | ((PDIV)<<8) | (MPLL_SDIV))

#define	PCLK_RATIO					(3)
#define	HCLKX2_RATIO				(0)
#define	HCLK_RATIO					(1)
#define	MPLL_RATIO					(0)
#define	ARM_RATIO					(0)

/* DDR regs */
#define P1MEMSTAT					(*(volatile unsigned long *)0x7E001000)
#define P1MEMCCMD					(*(volatile unsigned long *)0x7E001004)
#define P1DIRECTCMD					(*(volatile unsigned long *)0x7E001008)
#define P1MEMCFG					(*(volatile unsigned long *)0x7E00100C)
#define P1REFRESH					(*(volatile unsigned long *)0x7E001010)
#define P1CASLAT					(*(volatile unsigned long *)0x7E001014)
#define P1T_DQSS					(*(volatile unsigned long *)0x7E001018)
#define P1T_MRD						(*(volatile unsigned long *)0x7E00101C)
#define P1T_RAS						(*(volatile unsigned long *)0x7E001020)
#define P1T_RC						(*(volatile unsigned long *)0x7E001024)
#define P1T_RCD						(*(volatile unsigned long *)0x7E001028)
#define P1T_RFC						(*(volatile unsigned long *)0x7E00102C)
#define P1T_RP						(*(volatile unsigned long *)0x7E001030)
#define P1T_RRD						(*(volatile unsigned long *)0x7E001034)
#define P1T_WR						(*(volatile unsigned long *)0x7E001038)
#define P1T_WTR						(*(volatile unsigned long *)0x7E00103C)
#define P1T_XP						(*(volatile unsigned long *)0x7E001040)
#define P1T_XSR						(*(volatile unsigned long *)0x7E001044)
#define P1T_ESR						(*(volatile unsigned long *)0x7E001048)
#define P1MEMCFG2					(*(volatile unsigned long *)0X7E00104C)
#define P1_chip_0_cfg				(*(volatile unsigned long *)0x7E001200)
#define MEM_SYS_CFG					(*(volatile unsigned long *)0x7E00F120)

#define	HCLK						(133000000)
#define NS2CLOCK(ns)				((ns)/((1000000000)/(HCLK)) + 1)

#endif


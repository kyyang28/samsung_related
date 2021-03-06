#ifndef __S3C6410_REGS_H__
#define __S3C6410_REGS_H__

/* LED */
#define GPMCON						(*(volatile unsigned long *)0x7F008820)
#define GPMDAT						(*(volatile unsigned long *)0x7F008824)
#define GPMPUD						(*(volatile unsigned long *)0x7F008828)

/* CLOCK */
#define APLL_LOCK					(*(volatile unsigned long *)0x7E00F000)
#define MPLL_LOCK					(*(volatile unsigned long *)0x7E00F004)
#define EPLL_LOCK					(*(volatile unsigned long *)0x7E00F008)
#define APLL_CON					(*(volatile unsigned long *)0x7E00F00C)
#define MPLL_CON					(*(volatile unsigned long *)0x7E00F010)
#define CLK_SRC						(*(volatile unsigned long *)0x7E00F01C)
#define	CLK_DIV0					(*(volatile unsigned long *)0x7E00F020)
#define OTHERS						(*(volatile unsigned long *)0x7E00F900)

/* 533MHz */
#define	MDIV						(266)
#define	PDIV						(3)
#define	APLL_SDIV					(1)
#define	MPLL_SDIV					(2)
#define	APLLCON_VAL					(1<<31 | ((MDIV)<<16) | ((PDIV)<<8) | (APLL_SDIV))
#define	MPLLCON_VAL					(1<<31 | ((MDIV)<<16) | ((PDIV)<<8) | (MPLL_SDIV))

#define	PCLK_RATIO					(3)
#define	HCLKX2_RATIO				(0)
#define	HCLK_RATIO					(1)
#define	MPLL_RATIO					(0)
#define	ARM_RATIO					(0)

#endif


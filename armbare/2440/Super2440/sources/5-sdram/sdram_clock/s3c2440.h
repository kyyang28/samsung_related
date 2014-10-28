
#ifndef __S3C2440_H__
#define __S3C2440_H__

/* Watchdog */
#define	WTCON				(*(volatile unsigned long *)0x53000000)

/* LED */
#define GPFCON                          (*(volatile unsigned long *)0x56000050)
#define GPFDAT                          (*(volatile unsigned long *)0x56000054)

#define GPGCON                          (*(volatile unsigned int *)0x56000060)
#define GPGDAT                          (*(volatile unsigned int *)0x56000064)

/* Clock */ 
#define	S3C2440_CLOCK_BASE		0x4c000000
#define MDIV				0x5c
#define PDIV				0x01
#define SDIV				0x01

/* SDRAM */
#define S3C2440_SDRAM_BASE		0x48000000

/* BWSCON */
#define DW8		 	(0x0)
#define DW16		 	(0x1)
#define DW32		 	(0x2)

#define B1_BWSCON	  	(DW16)
#define B2_BWSCON	  	(DW16)
#define B3_BWSCON	  	(DW16)
#define B4_BWSCON	  	(DW16)
#define B5_BWSCON	  	(DW8)
#define B6_BWSCON	  	(DW32)
#define B7_BWSCON	  	(DW32)

/* BANKCON0 */
#define B0_Tacs			(0x0) /* 0 clock */
#define B0_Tcos			(0x0) /* 0 clock */
#define B0_Tacc			(0x7) /* 14 clock */
#define B0_Tcoh			(0x0) /* 0 clock */
#define B0_Tcah			(0x0) /* 0 clock */
#define B0_Tacp			(0x0) /* 2 clock */
#define B0_PMC			(0x0) /* normal */

/* BANKCON1 */
#define B1_Tacs			(0x0) /* 0 clock */
#define B1_Tcos			(0x0) /* 0 clock */
#define B1_Tacc			(0x7) /* 14 clock */
#define B1_Tcoh			(0x0) /* 0 clock */
#define B1_Tcah			(0x0) /* 0 clock */
#define B1_Tacp			(0x0) /* 2 clock */
#define B1_PMC			(0x0) /* normal */

/* BANKCON2 */
#define B2_Tacs			(0x0) /* 0 clock */
#define B2_Tcos			(0x0) /* 0 clock */
#define B2_Tacc			(0x7) /* 14 clock */
#define B2_Tcoh			(0x0) /* 0 clock */
#define B2_Tcah			(0x0) /* 0 clock */
#define B2_Tacp			(0x0) /* 2 clock */
#define B2_PMC			(0x0) /* normal */

/* BANKCON3 */
#define B3_Tacs			(0x0) /* 0 clock */
#define B3_Tcos			(0x0) /* 0 clock */
#define B3_Tacc			(0x7) /* 14 clock */
#define B3_Tcoh			(0x0) /* 0 clock */
#define B3_Tcah			(0x0) /* 0 clock */
#define B3_Tacp			(0x0) /* 2 clock */
#define B3_PMC			(0x0) /* normal */

/* BANKCON4 */
#define B4_Tacs			(0x0) /* 0 clock */
#define B4_Tcos			(0x0) /* 0 clock */
#define B4_Tacc			(0x7) /* 14 clock */
#define B4_Tcoh			(0x0) /* 0 clock */
#define B4_Tcah			(0x0) /* 0 clock */
#define B4_Tacp			(0x0) /* 2 clock */
#define B4_PMC			(0x0) /* normal */

/* BANKCON5 */
#define B5_Tacs			(0x0) /* 0 clock */
#define B5_Tcos			(0x0) /* 0 clock */
#define B5_Tacc			(0x7) /* 14 clock */
#define B5_Tcoh			(0x0) /* 0 clock */
#define B5_Tcah			(0x0) /* 0 clock */
#define B5_Tacp			(0x0) /* 2 clock */
#define B5_PMC			(0x0) /* normal */

/* BANKCON6 */
#define B6_MT			(0x3) /* SDRAM */
#define B6_Trcd			(0x1) /* 3 clocks */
#define B6_SCAN			(0x1) /* Column address 9-bit */

/* BANKCON7 */
#define B7_MT			(0x3) /* SDRAM */
#define B7_Trcd			(0x1) /* 3 clocks */
#define B7_SCAN			(0x1) /* Column address 9-bit */

/* REFRESH */
#define REFEN			(0x1) /* Refresh enable */
#define TREFMD			(0x0) /* CBR/Auto Refresh */
#define Trp			(0x0) /* SDRAM RAS pre-charge Time - 2 clocks */
#define Tsrc			(0x3) /* SDRAM Semi Row cycle time - 7 clocks */
#define REFCNT			(0x4F4) 
				/* Refresh period = (2^11-refresh_count+1)/HCLK */
				/* 8192 refresh cycles / 64ms = 7.8125 us
				   If HCLK = 12MHz: 
				      REFCNT = 2^11 + 1 - 7.8125*12 = 1955 = 0x7A3  
				   If HCLK = 100MHz:
				      REFCNT = 2^11 + 1 - 7.8125*100 = 1268 = 0x4F4 
				 */

/* BANKSIZE */
#define BURST_EN		(0x1) /* Enable burst operation */
#define SCKE_EN			(0x1) /* SDRAM power down mode enable */
#define SCLK_EN			(0x1) /* SCLK is active only during the access */
#define BK76MAP			(0x1) /* 64MB/64MB */

/* MRSRB6 */
#define WBL			(0x0) /* Write burst length - Burst (Fixed) */
#define TM			(0x0) /* Test mode - Mode register set (Fixed) */
#define CL			(0x3) /* CAS latency - 3 clocks */
#define	BT			(0x0) /* Burst type - Sequential (Fixed) */
#define BL			(0x0) /* Burst length - 1 (Fixed) */

/* MRSRB7 */
#define WBL			(0x0) /* Write burst length - Burst (Fixed) */
#define TM			(0x0) /* Test mode - Mode register set (Fixed) */
#define CL			(0x3) /* CAS latency - 3 clocks */
#define	BT			(0x0) /* Burst type - Sequential (Fixed) */
#define BL			(0x0) /* Burst length - 1 (Fixed) */


typedef unsigned int	u32;

typedef volatile u32	S3C2440_REG32;


/* CLOCK Registers  */
typedef struct {
	S3C2440_REG32 LOCKTIME;
	S3C2440_REG32 MPLLCON;
	S3C2440_REG32 UPLLCON;
	S3C2440_REG32 CLKCON;
	S3C2440_REG32 CLKSLOW;
	S3C2440_REG32 CLKDIVN;
	S3C2440_REG32 CAMDIVN;
}/*__attribute__((__packed__)) */S3C2440_CLOCK;


/* SDRAM Registers */
typedef struct {
	S3C2440_REG32 BWSCON;
	S3C2440_REG32 BANKCON0;
	S3C2440_REG32 BANKCON1;
	S3C2440_REG32 BANKCON2;
	S3C2440_REG32 BANKCON3;
	S3C2440_REG32 BANKCON4;
	S3C2440_REG32 BANKCON5;
	S3C2440_REG32 BANKCON6;
	S3C2440_REG32 BANKCON7;
	S3C2440_REG32 REFRESH;
	S3C2440_REG32 BANKSIZE;
	S3C2440_REG32 MRSRB6;
	S3C2440_REG32 MRSRB7;
}/*__attribute__((__packed__)) */ S3C2440_SDRAM;


static inline S3C2440_SDRAM * const S3C2440_Get_SDRAM_Base(void)
{
	return (S3C2440_SDRAM * const)S3C2440_SDRAM_BASE;
}


static inline S3C2440_CLOCK * const S3C2440_Get_CLOCK_Base(void)
{
	return (S3C2440_CLOCK * const)S3C2440_CLOCK_BASE;
}

#endif


#ifndef __S3C2440_REGS_H
#define __S3C2440_REGS_H

/* GPIO registers */
/* GPIO for buzzer and leds */
#define GPBCON								(*(volatile unsigned long *)0x56000010)
#define GPBDAT								(*(volatile unsigned long *)0x56000014)
#define GPBUP								(*(volatile unsigned long *)0x56000018)

/* GPIO for IIC */
#define GPECON								(*(volatile unsigned long *)0x56000040)
#define GPEDAT								(*(volatile unsigned long *)0x56000044)
#define GPEUP								(*(volatile unsigned long *)0x56000048)

/* GPIO for KEYS */
#define GPGCON								(*(volatile unsigned long *)0x56000060)
#define GPGDAT								(*(volatile unsigned long *)0x56000064)
#define GPGUP								(*(volatile unsigned long *)0x56000068)

/* GPIO for UART */
#define GPHCON								(*(volatile unsigned long *)0x56000070)
#define GPHDAT								(*(volatile unsigned long *)0x56000074)
#define GPHUP								(*(volatile unsigned long *)0x56000078)

/* Clock */
#define CLOCK_BASE_ADDR						(0x4C000000)

#define LOCKTIME_OFS						(0x00)
#define MPLLCON_OFS							(0x04)
#define CLKDIVN_OFS							(0x14)

#define	CLKDIVN_VALS						(0x05)	/* 1:4:8 */
#define MINI2440_MPLLCON_405MHZ				((0x7F<<12) | (0x2<<4) | (0x1))


/* SDRAM */
#define MEMORY_CONTROLLER_BASE_ADDR					(0x48000000)

#define BWSCON_OFS									(0x00)
#define BANKCON6_OFS								(0x1C)
#define REFRESH_OFS									(0x24)
#define BANKSIZE_OFS								(0x28)
#define MRSRB6_OFS									(0x2C)

#define BWSCON_VALS									(0x02<<24)
#define BANKCON6_VALS								(0x03<<15 | 0x01<<2 | 0x01)
#define REFRESH_VALS								(0x01<<23 | 0x00<<22 | 0x00<<20 | 0x03<<18 | 0x4F4)
#define BANKSIZE_VALS								(0x01<<7 | 0x01<<4 | 0x01)
#define MRSRB6_VALS									(0x03<<4)

/* Nandflash */
#define NFCONF										(*(volatile unsigned long *)0x4E000000)
#define NFCONT										(*(volatile unsigned long *)0x4E000004)
#define NFCMMD										(*(volatile unsigned char *)0x4E000008)
#define NFADDR										(*(volatile unsigned char *)0x4E00000C)
#define NFDATA										(*(volatile unsigned char *)0x4E000010)
#define NFSTAT										(*(volatile unsigned char *)0x4E000020)

#define NAND_NFCONF_TACL							0
#define NAND_NFCONF_TWRPH0							1
#define NAND_NFCONF_TWRPH1							0

#define NAND_NFCONT_INITECC							(1<<4)
#define NAND_NFCONT_CHIP_DISABLE					(1<<1)
#define NAND_NFCONT_CHIP_ENABLE						(0<<1)
#define NAND_NFCONT_CONTROLLER_ENABLE				(1<<0)
#define NAND_NFCONT_CONTROLLER_DISABLE				(0<<0)

#define NAND_NFSTAT_STATUS							(0x01)

#define NAND_PAGE_SIZE								2048

/* PWM & Timer registers */
#define	TCFG0								(*(volatile unsigned long *)0x51000000)
#define	TCFG1								(*(volatile unsigned long *)0x51000004)
#define	TCON								(*(volatile unsigned long *)0x51000008)
#define	TCNTB0								(*(volatile unsigned long *)0x5100000C)
#define	TCMPB0								(*(volatile unsigned long *)0x51000010)
#define	TCNTO0								(*(volatile unsigned long *)0x51000014)
#define	TCNTB1								(*(volatile unsigned long *)0x51000018)
#define	TCMPB1								(*(volatile unsigned long *)0x5100001C)
#define	TCNTO1								(*(volatile unsigned long *)0x51000020)
#define	TCNTB2								(*(volatile unsigned long *)0x51000024)
#define	TCMPB2								(*(volatile unsigned long *)0x51000028)
#define	TCNTO2								(*(volatile unsigned long *)0x5100002C)
#define	TCNTB3								(*(volatile unsigned long *)0x51000030)
#define	TCMPB3								(*(volatile unsigned long *)0x51000034)
#define	TCNTO3								(*(volatile unsigned long *)0x51000038)
#define	TCNTB4								(*(volatile unsigned long *)0x5100003C)
#define	TCNTO4								(*(volatile unsigned long *)0x51000040)

/* RTC */
#define RTCCON								(*(volatile unsigned char *)0x57000040)
#define TICNT								(*(volatile unsigned char *)0x57000044)
#define RTCALM								(*(volatile unsigned char *)0x57000050)
#define ALMSEC								(*(volatile unsigned char *)0x57000054)
#define ALMMIN								(*(volatile unsigned char *)0x57000058)
#define ALMHOUR								(*(volatile unsigned char *)0x5700005C)
#define ALMDATE								(*(volatile unsigned char *)0x57000060)
#define ALMMON								(*(volatile unsigned char *)0x57000064)
#define ALMYEAR								(*(volatile unsigned char *)0x57000068)
#define BCDSEC								(*(volatile unsigned char *)0x57000070)
#define BCDMIN								(*(volatile unsigned char *)0x57000074)
#define BCDHOUR								(*(volatile unsigned char *)0x57000078)
#define BCDDATE								(*(volatile unsigned char *)0x5700007C)
#define BCDDAY								(*(volatile unsigned char *)0x57000080)
#define BCDMON								(*(volatile unsigned char *)0x57000084)
#define BCDYEAR								(*(volatile unsigned char *)0x57000088)

#define RTCCON_RTC_ENABLE					(1)
#define RTCCON_RTC_DISABLE					(0)

/* UART */
/* UART registers */
#define ULCON0								(*(volatile unsigned long *)0x50000000)
#define UCON0								(*(volatile unsigned long *)0x50000004)
#define UFCON0								(*(volatile unsigned long *)0x50000008)
#define UTRSTAT0							(*(volatile unsigned long *)0x50000010)
#define UFSTAT0								(*(volatile unsigned long *)0x50000018)
#define UTXH0								(*(volatile unsigned char *)0x50000020)
#define URXH0								(*(volatile unsigned char *)0x50000024)
#define UBRDIV0								(*(volatile unsigned long *)0x50000028)

#define PCLK								(50000000)
#define BAUDRATE							(115200)
#define UBRDIV0_VALS						((PCLK / (BAUDRATE * 16)) - 1)

/* Interrupt Controller */
#define SRCPND								(*(volatile unsigned long *)0x4A000000)
#define INTMOD								(*(volatile unsigned long *)0x4A000004)
#define INTMSK								(*(volatile unsigned long *)0x4A000008)
#define PRIORITY							(*(volatile unsigned long *)0x4A00000C)
#define INTPND								(*(volatile unsigned long *)0x4A000010)
#define INTOFFSET							(*(volatile unsigned long *)0x4A000014)
#define SUBSRCPND							(*(volatile unsigned long *)0x4A000018)
#define INTSUBMSK							(*(volatile unsigned long *)0x4A00001C)

/* External Interrupt for keys */
#define EXTINT1								(*(volatile unsigned long *)0x5600008C)
#define EXTINT2								(*(volatile unsigned long *)0x56000090)
#define EINTMASK							(*(volatile unsigned long *)0x560000A4)
#define EINTPEND							(*(volatile unsigned long *)0x560000A8)

/* INTOFFSET */
#define EINT8_23_OFT						(5)
#define INT_TIMER0_OFT						(10)
#define INT_IIC_OFT							(27)
#define INT_RTC_OFT							(30)
#define INT_ADC_OFT							(31)

/* PENDING BITS */
#define INT_ADC								(0x1<<31)
#define INT_RTC								(0x1<<30)
#define INT_IIC								(0x1<<27)
#define INT_ADC_S							(0x1<<10)
#define INT_TIMER0							(0x1<<10)
#define INT_TC								(0x1<<9)
#define EINT8_23							(0x1<<5)

/* A/D Converter */
#define ADCCON								(*(volatile unsigned long *)0x58000000)
#define ADCTSC								(*(volatile unsigned long *)0x58000004)
#define ADCDLY								(*(volatile unsigned long *)0x58000008)
#define ADCDAT0								(*(volatile unsigned long *)0x5800000C)
#define ADCDAT1								(*(volatile unsigned long *)0x58000010)
#define ADCUPDN								(*(volatile unsigned long *)0x58000014)

#define ADC_FREQ							2500000

/* IIC */
#define IICCON								(*(volatile unsigned long *)0x54000000)
#define IICSTAT								(*(volatile unsigned long *)0x54000004)
#define IICADD								(*(volatile unsigned long *)0x54000008)
#define IICDS								(*(volatile unsigned long *)0x5400000C)
#define IICLC								(*(volatile unsigned long *)0x54000010)

#define RD_DATA								(0)
#define WR_DATA								(1)

/* IIS */
#define IISCON								(*(volatile unsigned long *)0x55000000)
#define IISMOD								(*(volatile unsigned long *)0x55000004)
#define IISPSR								(*(volatile unsigned long *)0x55000008)
#define IISFCON								(*(volatile unsigned long *)0x5500000C)
#define IISFIFO								(*(volatile unsigned long *)0x55000010)

/* DMA */
#define DISRC2								(*(volatile unsigned long *)0x4B000080)
#define DISRCC2								(*(volatile unsigned long *)0x4B000084)
#define DIDST2								(*(volatile unsigned long *)0x4B000088)
#define DIDSTC2								(*(volatile unsigned long *)0x4B00008C)
#define DCON2								(*(volatile unsigned long *)0x4B000090)
#define DMASKTRIG2							(*(volatile unsigned long *)0x4B0000A0)

/* SD */
#define SDICON							(*(volatile unsigned long *)0x5A000000)	//SDI control
#define SDIPRE							(*(volatile unsigned long *)0x5A000004)	//SDI baud rate prescaler
#define SDICARG							(*(volatile unsigned long *)0x5A000008)	//SDI command argument
#define SDICCON							(*(volatile unsigned long *)0x5A00000C)	//SDI command control
#define SDICSTA							(*(volatile unsigned long *)0x5A000010)	//SDI command status
#define SDIRSP0							(*(volatile unsigned long *)0x5A000014)	//SDI response 0
#define SDIRSP1							(*(volatile unsigned long *)0x5A000018)	//SDI response 1
#define SDIRSP2							(*(volatile unsigned long *)0x5A00001C)	//SDI response 2
#define SDIRSP3							(*(volatile unsigned long *)0x5A000020)	//SDI response 3
#define SDIDTIMER						(*(volatile unsigned long *)0x5A000024)	//SDI data/busy timer
#define SDIBSIZE						(*(volatile unsigned long *)0x5A000028)	//SDI block size
#define SDIDCON							(*(volatile unsigned long *)0x5A00002C)	//SDI data control
#define SDIDCNT							(*(volatile unsigned long *)0x5A000030)	//SDI data remain counter
#define SDIDSTA							(*(volatile unsigned long *)0x5A000034)	//SDI data status
#define SDIFSTA							(*(volatile unsigned long *)0x5A000038)	//SDI FIFO status
#define SDIIMSK							(*(volatile unsigned long *)0x5A00003C)	//SDI interrupt mask. 
#define SDIDAT							(*(volatile unsigned long *)0x5A000040)	//SDI data 


/* Types */
#define u8									unsigned char
#define u16									unsigned short
#define u32									unsigned int

#define NULL								0

#define DELAY(t)	do { \
						int i; \
						for (i = 0; i < t; i++); \
					} while(0)

#endif


/**************************************************************************************
* 
*	Project Name : S3C6410 Validation
*
*	Copyright 2006 by Samsung Electronics, Inc.
*	All rights reserved.
*
*	Project Description :
*		This software is only for validating functions of the S3C6410.
*		Anybody can use this software without our permission.
*  
*--------------------------------------------------------------------------------------
* 
*	File Name : intc.h
*  
*	File Description : This file declares prototypes of interrupt controller API funcions.
*
*	Author : Haksoo,Kim
*	Dept. : AP Development Team
*	Created Date : 2006/11/08
*	Version : 0.1 
* 
*	History
*	- Created(Haksoo,Kim 2006/11/08)
*	- Added Software Interrupt API function (wonjoon.jang 2007/01/18)
*     - Added Interrupt Source for 6410  (2008/02/28)
*  
**************************************************************************************/

#ifndef __INTC_H__
#define __INTC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "def.h"
#include "sfr6410.h"

void INTC_Init(void);
void INTC_ClearVectAddr(void);

#define INT_LIMIT				(64)

//INT NUM - VIC0
#define NUM_EINT0				(0)
#define NUM_EINT1				(1)
#define NUM_RTC_TIC				(2)
#define NUM_CAMIF_C			(3)
#define NUM_CAMIF_P			(4)
#define NUM_I2C1				(5)
#define NUM_I2S					(6)

#define NUM_3D					(8)
#define NUM_POST0				(9)
#define NUM_ROTATOR			(10)
#define NUM_2D					(11)
#define NUM_TVENC				(12)
#define NUM_SCALER				(13)
#define NUM_BATF				(14)
#define NUM_JPEG				(15)
#define NUM_MFC					(16)
#define NUM_SDMA0				(17)
#define NUM_SDMA1				(18)
#define NUM_ARM_DMAERR		(19)
#define NUM_ARM_DMA			(20)
#define NUM_ARM_DMAS			(21)
#define NUM_KEYPAD				(22)
#define NUM_TIMER0				(23)
#define NUM_TIMER1				(24)
#define NUM_TIMER2				(25)
#define NUM_WDT				(26)
#define NUM_TIMER3				(27)
#define NUM_TIMER4				(28)
#define NUM_LCD0				(29)
#define NUM_LCD1				(30)
#define NUM_LCD2				(31)

//INT NUM - VIC1
#define NUM_EINT2				(32+0)
#define NUM_EINT3				(32+1)
#define NUM_PCM0				(32+2)
#define NUM_PCM1				(32+3)
#define NUM_AC97				(32+4)
#define NUM_UART0				(32+5)
#define NUM_UART1				(32+6)
#define NUM_UART2				(32+7)
#define NUM_UART3				(32+8)
#define NUM_DMA0				(32+9)
#define NUM_DMA1				(32+10)
#define NUM_ONENAND0			(32+11)
#define NUM_ONENAND1			(32+12)
#define NUM_NFC					(32+13)
#define NUM_CFC					(32+14)
#define NUM_UHOST				(32+15)
#define NUM_SPI0				(32+16)
#define NUM_SPI1				(32+17)
#define NUM_IIC					(32+18)
#define NUM_HSItx				(32+19)
#define NUM_HSIrx				(32+20)
#define NUM_EINTGroup			(32+21)
#define NUM_MSM				(32+22)
#define NUM_HOSTIF				(32+23)
#define NUM_HSMMC0				(32+24)
#define NUM_HSMMC1				(32+25)
#define NUM_OTG					(32+26)
#define NUM_IRDA				(32+27)
#define NUM_RTC_ALARM			(32+28)
#define NUM_SEC					(32+29)
#define NUM_PENDNUP				(32+30)
#define NUM_ADC					(32+31)
#define NUM_PMU					(32+32)

// VIC0
#define	rVIC0IRQSTATUS			(VIC0_BASE + 0x00)
#define	rVIC0FIQSTATUS			(VIC0_BASE + 0x04)
#define	rVIC0RAWINTR			(VIC0_BASE + 0x08)
#define	rVIC0INTSELECT			(VIC0_BASE + 0x0c)
#define	rVIC0INTENABLE			(VIC0_BASE + 0x10)
#define	rVIC0INTENCLEAR			(VIC0_BASE + 0x14)
#define	rVIC0SOFTINT			(VIC0_BASE + 0x18)
#define	rVIC0SOFTINTCLEAR		(VIC0_BASE + 0x1c)
#define	rVIC0PROTECTION			(VIC0_BASE + 0x20)
#define	rVIC0SWPRIORITYMASK		(VIC0_BASE + 0x24)
#define	rVIC0PRIORITYDAISY		(VIC0_BASE + 0x28)

#define rVIC0VECTADDR			(VIC0_BASE + 0x100)

#define rVIC0VECPRIORITY		(VIC0_BASE + 0x200)

#define rVIC0ADDR				(VIC0_BASE + 0xf00)
#define rVIC0PERID0				(VIC0_BASE + 0xfe0)
#define rVIC0PERID1				(VIC0_BASE + 0xfe4)
#define rVIC0PERID2				(VIC0_BASE + 0xfe8)
#define rVIC0PERID3				(VIC0_BASE + 0xfec)
#define rVIC0PCELLID0			(VIC0_BASE + 0xff0)
#define rVIC0PCELLID1			(VIC0_BASE + 0xff4)
#define rVIC0PCELLID2			(VIC0_BASE + 0xff8)
#define rVIC0PCELLID3			(VIC0_BASE + 0xffc)

// VIC1
#define	rVIC1IRQSTATUS			(VIC1_BASE + 0x00)
#define	rVIC1FIQSTATUS			(VIC1_BASE + 0x04)
#define	rVIC1RAWINTR			(VIC1_BASE + 0x08)
#define	rVIC1INTSELECT			(VIC1_BASE + 0x0c)
#define	rVIC1INTENABLE			(VIC1_BASE + 0x10)
#define	rVIC1INTENCLEAR			(VIC1_BASE + 0x14)
#define	rVIC1SOFTINT			(VIC1_BASE + 0x18)
#define	rVIC1SOFTINTCLEAR		(VIC1_BASE + 0x1c)
#define	rVIC1PROTECTION			(VIC1_BASE + 0x20)
#define	rVIC1SWPRIORITYMASK		(VIC1_BASE + 0x24)
#define	rVIC1PRIORITYDAISY		(VIC1_BASE + 0x28)

#define rVIC1VECTADDR			(VIC1_BASE + 0x100)

#define rVIC1VECPRIORITY		(VIC1_BASE + 0x200)

#define rVIC1ADDR				(VIC1_BASE + 0xf00)
#define rVIC1PERID0				(VIC1_BASE + 0xfe0)
#define rVIC1PERID1				(VIC1_BASE + 0xfe4)
#define rVIC1PERID2				(VIC1_BASE + 0xfe8)
#define rVIC1PERID3				(VIC1_BASE + 0xfec)
#define rVIC1PCELLID0			(VIC1_BASE + 0xff0)
#define rVIC1PCELLID1			(VIC1_BASE + 0xff4)
#define rVIC1PCELLID2			(VIC1_BASE + 0xff8)
#define rVIC1PCELLID3			(VIC1_BASE + 0xffc)

#ifdef __cplusplus
}
#endif

#endif 



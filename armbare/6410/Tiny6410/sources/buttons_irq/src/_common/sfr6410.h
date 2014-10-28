/**************************************************************************************
* 
*	Project Name : S3C6410 Validation
*
*	Copyright 2006~2008 by Samsung Electronics, Inc.
*	All rights reserved.
*
*	Project Description :
*		This software is only for validating functions of the S3C6410.
*		Anybody can use this software without our permission.
*  
*--------------------------------------------------------------------------------------
* 
*	File Name : sfr6410.h
*  
*	File Description : This file defines SFR base addresses.
*
*	Author : Haksoo,Kim
*	Dept. : AP Development Team
*	Created Date : 2006/11/08
*	Version : 0.1 
* 
*	History
*	- Created(Haksoo,Kim 2006/11/08)
*	- Added - AHB_S  (wonjoon.jang 2007/1/2 ) 
*	- Added I2C0, I2C1 (OnPil, Shin (SOP) 2008/03/01)
*  
**************************************************************************************/


#ifndef __sfr6410_H__
#define __sfr6410_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "def.h"

////
//AHB_SMC
//
//SMC
#define SROM_BASE				(0x70000000)	//SROM
#define ONENAND0_BASE			(0x70100000)	//OneNAND
#define ONENAND1_BASE			(0x70180000)	//OneNAND
#define NFCON_BASE				(0x70200000)	//Nand Flash
#define CFCON_BASE				(0x70300000)	//CF


////
//TZIC
//
//TZIC
#define TZIC0_BASE				(0x71000000)
#define TZIC1_BASE				(0x71100000)


////
//VIC
//
//VIC
#define VIC0_BASE				(0x71200000)
#define VIC1_BASE				(0x71300000)


////
//3D
//
//3D
#define FIMG_BASE   				(0x72000000)




////
//AHB_ETB
//
//ETB
#define ETBMEM_BASE				(0x73000000)
#define ETBSFR_BASE				(0x73100000)


////
//AHB_T
//
//HOST i/f
#define HOSTIF_SFR_BASE			(0x74000000)
#define DPSRAM_BASE				(0x74100000)
#define MODEMIF_BASE			(0x74108000)
//USB HOST
#define USBHOST_BASE			(0x74300000)
#define MDPIF_BASE				(0x74400000)


////
//AHB_M
//
//DMA
#define DMA0_BASE				(0x75000000)
#define DMA1_BASE				(0x75100000)


////
//AHB_P
//
// 2D
#define G2D_BASE				(0x76100000)
//TV
#define TVENC_BASE				(0x76200000)
#define TVSCALER_BASE			(0X76300000)


////
//AHB_F
//
//POST
#define POST0_BASE				(0x77000000)
//LCD
#define LCD_BASE				(0x77100000)
//ROTATOR
#define ROTATOR_BASE			(0x77200000)


////
//AHB_I
//
//CAMERA I/F
#define CAMERA_BASE				(0x78000000)
//JPEG
#define JPEG_BASE				(0x78800000)


////
//AHB_X
//
//USB OTG
#define USBOTG_LINK_BASE		(0x7C000000)
#define USBOTG_PHY_BASE			(0x7C100000)
//HS MMC
#define HSMMC0_BASE				(0x7C200000)
#define HSMMC1_BASE				(0x7C300000)
#define HSMMC2_BASE				(0x7C400000)


////
//AHB_S
//
//D&I Security Sub System Base
#define DnI_BASE				(0x7D000000)
#define AES_RX_BASE				(0x7D100000)
#define DES_RX_BASE				(0x7D200000)
#define HASH_RX_BASE			(0x7D300000)
#define RX_SFR_BASE				(0x7D400000)
#define AES_TX_BASE				(0x7D500000)
#define DES_TX_BASE				(0x7D600000)
#define HASH_TX_BASE			(0x7D700000)
#define TX_SFR_BASE				(0x7D800000)
#define RX_FIFO_BASE			(0x7D900000)
#define TX_FIFO_BASE			(0x7DA00000)
// SDMA Controller
#define SDMA0_BASE				(0x7DB00000)
#define SDMA1_BASE				(0x7DC00000)


////
//APB0
//
//DMC
#define DMC0_BASE				(0x7E000000)
#define DMC1_BASE				(0x7E001000)
//MFC
#define MFC_BASE				(0x7E002000)
//WDT
#define WDT_BASE				(0x7E004000)
//RTC
#define RTC_BASE				(0x7E005000)
//HSI
#define HSITX_BASE				(0x7E006000)
#define HSIRX_BASE				(0x7E007000)
//KEYPAD I/F
#define KEYPADIF_BASE			(0x7E00A000)
//ADC TS
#define ADCTS_BASE				(0x7E00B000)
//ETM
#define ETM_BASE				(0x7E00C000)
//KEY
#define KEY_BASE				(0x7E00D000)
//Chip ID
#define CHIPID_BASE				(0x7E00E000)
//SYSCON
#define SYSCON_BASE				(0x7E00F000)


////
//APB1
//
//TZPC
#define TZPC_BASE				(0x7F000000)
//AC97
#define AC97_BASE				(0x7F001000)
//I2S
#define I2S0_BASE				(0x7F002000)
#define I2S1_BASE				(0x7F003000)
//I2C (Channel0, 1 Added by SOP on 2008.03.01)
#define I2C0_BASE				(0x7F004000)
#define I2C1_BASE				(0x7F00F000)
//UART
#define UART_BASE				(0x7F005000)
//PWM TIMER
#define PWMTIMER_BASE			(0x7F006000)
//IRDA
#define IRDA_BASE				(0x7F007000)
//GPIO
#define GPIO_BASE				(0x7F008000)
//PCM
#define PCM0_BASE				(0x7F009000)
#define PCM1_BASE				(0x7F00A000)
//SPI
#define SPI0_BASE				(0x7F00B000)
#define SPI1_BASE				(0x7F00C000)
//I2S MULTI
#define I2SMULTI_BASE			(0X7F00D00)

//GIB
#define GIB_BASE					(0x7F00E000)


#ifdef __cplusplus
}
#endif

#endif /*__sfr6410_H__*/

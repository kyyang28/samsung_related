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
*	File Name : system.h
*  
*	File Description : This file declares prototypes of system funcions.
*
*	Author : Haksoo,Kim
*	Dept. : AP Development Team
*	Created Date : 2006/11/08
*	Version : 0.1 
* 
*	History
*	- Created(Haksoo,Kim 2006/11/08)
*  
**************************************************************************************/

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "def.h"

extern u8	g_System_Revision, g_System_Pass;
extern u32	g_APLL, g_MPLL, g_ARMCLK, g_HCLKx2, g_HCLK, g_PCLK;

// Camera module define by jungil 01.31
#define CAM_OV7620			1
#define CAM_S5X433			2
#define	CAM_AU70H			3
#define CAM_S5X3A1			4
#define CAM_S5K3AA			5
#define CAM_S5K3BA			6
#define	CAM_S5K4AAF			7
#define CAM_LCD_INTERLACE	8
#define CAM_LCD_PROGRESSIVE	9
#define CAM_A3AFX_VGA		10

#define CAM_MODEL			CAM_S5K3BA
//----------------------------------------


void SYSTEM_EnableVIC( void);
void SYSTEM_DisableVIC( void);
void SYSTEM_EnableIRQ( void);
void SYSTEM_DisableIRQ( void);
void SYSTEM_EnableFIQ( void);
void SYSTEM_DisableFIQ( void);

void SYSTEM_InitException( void);
void MMU_WaitForInterrupt(void);
void Stop_WFI_Test(void);
void Stop_WFI_Test1(void);

void SYSTEM_EnableBP(void);
void SYSTEM_EnableICache( void);
void SYSTEM_DisableICache( void);
void SYSTEM_EnableDCache( void);
void SYSTEM_DisableDCache( void);
void SYSTEM_InvalidateEntireICache( void);
void SYSTEM_InvalidateEntireDCache( void);
void SYSTEM_InvalidateBothCache( void);
void SYSTEM_CleanEntireDCache( void);
void SYSTEM_CleanInvalidateEntireDCache( void);
void SYSTEM_EnableMMU( void);
void SYSTEM_DisableMMU( void);
void SYSTEM_InvalidateTLB( void);
void SYSTEM_SetTTBase( u32);
void SYSTEM_SetDomain( u32);
void SYSTEM_SetFCSEPID( u32);
void SYSTEM_EnableAlignFault( void);
void SYSTEM_DisableAlignFault( void);
u32 SYSTEM_ReadDFSR( void);
u32 SYSTEM_ReadIFSR( void);
u32 SYSTEM_ReadFAR( void);

void SYSTEM_InitMmu(void);

void MEMCOPY_TEST(void);
void MEMCOPY_TEST0(void);
void MEMCOPY8 (u32 , u32, u32 );
void MEMWRITE4 (u32 , u32, u32 , u32); // addr, size, data1, data2

#ifdef __cplusplus
}
#endif

#endif /*__SYSTEM_H__*/

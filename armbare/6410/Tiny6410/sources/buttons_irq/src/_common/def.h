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
*	File Name : def.h
*  
*	File Description : This file defines some types used commonly.
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

#ifndef __DEF_H__
#define __DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

// Type defines 
typedef unsigned long		u32;
typedef unsigned short		u16;
typedef unsigned char		u8;

typedef signed long			s32;
typedef signed short		s16;
typedef signed char			s8;

#define FALSE			(0)
#define TRUE				(1)
#define false				(0)
#define true				(1)

#define Getc				UART_Getc
#define Putc				UART_Putc
#define GetKey				UART_GetKey
#define Disp				UART_Printf

typedef enum Function_Test
{
	eTest_Manual, eTest_Auto
}eFunction_Test;

typedef struct FunctionT_AutoVar
{
	u32 Test_Parameter[10];
}oFunctionT_AutoVar;

typedef struct testFuncMenu {
	void (*func)(void); 
	const char *desc;
} testFuncMenu;

typedef struct AutotestFuncMenu {
	void (*func)(eFunction_Test eTest, oFunctionT_AutoVar oParameter); 
	const char *desc;
} AutotestFuncMenu;

typedef enum
{
	PAL1, PAL2, PAL4, PAL8,
	RGB8, ARGB8, RGB16, ARGB16, RGB18, RGB24, RGB30, ARGB24,
	YC420, YC422, // Non-interleave
	CRYCBY, CBYCRY, YCRYCB, YCBYCR, YUV444, // Interleave
	RGBA16, RGBX24, RGBA24
} CSPACE;

typedef enum
{
	QCIF, CIF/*352x288*/, 
	QQVGA, QVGA, VGA, SVGA/*800x600*/, SXGA/*1280x1024*/, UXGA/*1600x1200*/, QXGA/*2048x1536*/,
	WVGA/*854x480*/, HD720/*1280x720*/, HD1080/*1920x1080*/
} IMG_SIZE;


typedef enum
{
	FLIP_NO, FLIP_X, FLIP_Y, FLIP_XY
} FLIP_DIR;

typedef enum
{
	ROT_0, ROT_90, ROT_180, ROT_270
} ROT_DEG;


typedef enum
{
	NTSC_M,
	PAL_M,
	PAL_BGHID,
	PAL_N,
	PAL_NC,
	PAL_60,
	NTSC_443,
	NTSC_J
} TV_STANDARDS;

typedef enum
{
	INTERLACE,
	PROGRESSIVE
} TV_SCAN_MODE;



typedef enum
{
	BT601, BT656
}ITU_R_STANDARD;

#if 0 
typedef enum
{	
	PROGRESSIVE,
	INTERLACE
} SCAN_MODE;
#endif

#ifdef __cplusplus
}
#endif

#endif


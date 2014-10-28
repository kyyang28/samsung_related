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
*	File Name : library.h
*  
*	File Description : This file defines the register access function
*						and declares prototypes of library funcions
*
*	Author : Haksoo,Kim
*	Dept. : AP Development Team
*	Created Date : 2006/11/08
*	Version : 0.1 
* 
*	History
*	- Created(Haksoo,Kim 2006/11/08)
*	- Added Pause() by OnPil, Shin on 2008/03/04
*  
**************************************************************************************/
#ifndef __LIBRARY_H__
#define __LIBRARY_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "def.h"


#define Outp32(addr, data)	(*(volatile u32 *)(addr) = (data))
#define Outp16(addr, data)	(*(volatile u16 *)(addr) = (data))
#define Outp8(addr, data)	(*(volatile u8 *)(addr) = (data))
#define Inp32(addr)			(*(volatile u32 *)(addr))
#if 1 // LCD SPISetting jungil
#define Inp32_SPI(addr, data) (data = (*(volatile u32 *)(addr)))
#endif

#define Inp16(addr)			(*(volatile u16 *)(addr))
#define Inp8(addr)			(*(volatile u8 *)(addr))


#ifdef __cplusplus
}
#endif

#endif /*__LIBRARY_H__*/


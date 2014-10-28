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
*	File Name : intc.c
*  
*	File Description : This file implements the API functons for interrupt controller.
*
*	Author : Haksoo,Kim
*	Dept. : AP Development Team
*	Created Date : 2006/11/08
*	Version : 0.1 
* 
*	History
*	- Created(Haksoo,Kim 2006/11/08)
*     - Added Software Interrupt API (wonjoon.jang 2007/01/17)
*  
**************************************************************************************/

#include "library.h"
#include "intc.h"

//////////
// Function Name : INTC_Init
// Function Description : This function initializes interrupt controller
// Input : NONE 
// Output : NONE
// Version : 
void INTC_Init(void)
{
#if (VIC_MODE==0)	
	u32 i;
	
	for(i=0;i<32;i++)
		Outp32(rVIC0VECTADDR+4*i, i);
	
	for(i=0;i<32;i++)
		Outp32(rVIC1VECTADDR+4*i, i+32);
#endif
	Outp32(rVIC0INTENCLEAR, 0xffffffff);
	Outp32(rVIC1INTENCLEAR, 0xffffffff);

	Outp32(rVIC0INTSELECT, 0x0);
	Outp32(rVIC1INTSELECT, 0x0);

	INTC_ClearVectAddr();

	return;
}

//////////
// Function Name : INTC_ClearVectAddr
// Function Description : This function clears the vector address register
// Input : NONE
// Output : NONE
// Version : 
void INTC_ClearVectAddr(void)
{
	Outp32(rVIC0ADDR, 0);
	Outp32(rVIC1ADDR, 0);
	
	return;
}


;/*************************************************************************************
; 
;	Project Name : S3C6400 Validation
;
;	Copyright 2006 by Samsung Electronics, Inc.
;	All rights reserved.
;
;	Project Description :
;		This software is only for validating functions of the S3C6400.
;		Anybody can use this software without our permission.
;  
;--------------------------------------------------------------------------------------
; 
;	File Name : option.inc
;  
;	File Description : This file defines basic setting and configuration.
;
;	Author	: Haksoo,Kim
;	Dept. : AP Development Team
;	Created Date : 2006/11/08
;	Version : 0.1 
; 
;	History
;	- Created(Haksoo,Kim 2006/11/08)
;  
;*************************************************************************************/

					GBLL	S3C6400
S3C6400				SETL	{TRUE}

					GBLL	SILICON
SILICON				SETL	{TRUE}

					GBLS	DMC
DMC					SETS	"DMC1"				
					
					GBLS	DRAM
DRAM				SETS	"DDR"									


 [ (DMC = "DMC0")
DRAM_BaseAddress	EQU		(0x40000000)
 ]
 [ (DMC = "DMC1")
DRAM_BaseAddress	EQU		(0x50000000)
 ] 
;top_of_stacks		EQU		(DRAM_BaseAddress+0x07ff8000)
;Exception_Vector	EQU		(DRAM_BaseAddress+0x07ffff00)

ONENAND_BaseAddress EQU		(0x70100000)
DownloadAddress		EQU		(DRAM_BaseAddress+0x00000000)

;rb1004.... 
;base_of_heap		EQU		(DRAM_BaseAddress+0x06000000)

base_of_heap		EQU		(DRAM_BaseAddress+0x04000000)
top_of_stacks		EQU		(DRAM_BaseAddress+0x07ff8000)
Exception_Vector	EQU		(DRAM_BaseAddress+0x07ffff00)

;rb1004
ITCM0_BaseAddress		EQU		0x80000011	;ITCM0 base : 0x80000000, Size : 8KB,
ITCM1_BaseAddress		EQU		0x80002011	;ITCM1 base : 0x80002000, Size : 8KB,
DTCM0_BaseAddress		EQU		0x80004011	;DTCM0 base : 0x80004000, Size : 8KB,
DTCM1_BaseAddress		EQU		0x80006011	;DTCM1 base : 0x80006000, Size : 8KB,

				END

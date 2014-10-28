;/*************************************************************************************
; 
;	Project Name : S3C6410 Validation
;
;	Copyright 2006 by Samsung Electronics, Inc.
;	All rights reserved.
;
;	Project Description :
;		This software is only for validating functions of the S3C6410.
;		Anybody can use this software without our permission.
;  
;--------------------------------------------------------------------------------------
; 
;	File Name : retarget_.s
;  
;	File Description : This file implements the stack & heap initilaization
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

				GET		option.inc
				

				AREA	|C$$code|, CODE, READONLY


Mode_USR		EQU		0x10
Mode_FIQ		EQU		0x11
Mode_IRQ		EQU		0x12
Mode_SVC		EQU		0x13
Mode_ABT		EQU		0x17
Mode_UND		EQU		0x1b
Mode_SYS		EQU		0x1f

I_Bit			EQU		0x80
F_Bit			EQU		0x40

Size_FIQ_Stack	EQU		256
Size_IRQ_Stack	EQU		256
Size_ABT_Stack	EQU		256
Size_UND_Stack	EQU		256
Size_SVC_Stack	EQU		8192


Offset_FIQ_Stack	EQU     0
Offset_IRQ_Stack	EQU     Offset_FIQ_Stack + Size_FIQ_Stack
Offset_ABT_Stack	EQU     Offset_IRQ_Stack + Size_IRQ_Stack
Offset_UND_Stack	EQU     Offset_ABT_Stack + Size_ABT_Stack
Offset_SVC_Stack	EQU     Offset_UND_Stack + Size_UND_Stack


				EXPORT	__user_initial_stackheap

__user_initial_stackheap

				ldr		r0,=top_of_stacks

				msr		CPSR_c,#Mode_FIQ:OR:I_Bit:OR:F_Bit
				sub		sp,r0,#Offset_FIQ_Stack

				msr		CPSR_c,#Mode_IRQ:OR:I_Bit:OR:F_Bit
				sub     sp,r0,#Offset_IRQ_Stack

				msr		CPSR_c,#Mode_ABT:OR:I_Bit:OR:F_Bit
				sub		sp,r0,#Offset_ABT_Stack

				msr		CPSR_c,#Mode_UND:OR:I_Bit:OR:F_Bit
				sub		sp,r0,#Offset_UND_Stack

				msr		CPSR_c,#Mode_SVC:OR:I_Bit:OR:F_Bit
				sub		r1,r0,#Offset_SVC_Stack
				
				;IMPORT  |Image$$ZI$$Limit|
				;LDR		r0, =|Image$$ZI$$Limit|					
				ldr		r0, =base_of_heap

				mov		r2,#0
				mov		r3,#0

				mov		pc,lr

;-------------------------------------------------------
	[	(VIC_MODE = 0)
				IMPORT	IntHandlerTable
				EXPORT	IsrIRQ
IsrIRQ
				sub		sp,sp,#4			; reserved for PC
				stmfd	sp!,{r8-r9}
				ldr		r9,=0x71200f00		; rVIC0ADDR
				ldr		r9,[r9]
				ldr		r8,=IntHandlerTable
				add		r8,r8,r9,lsl #2
				ldr		r8,[r8]
				str		r8,[sp,#8]
				ldmfd	sp!,{r8-r9,pc}
	]


				LTORG
   
				END
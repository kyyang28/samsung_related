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
;	File Name : system_.s
;  
;	File Description : This file implements CP15 control.
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


				AREA	|C$$code|, CODE, READONLY
;;===============================================================================
;;
;;	Exception
;;
						EXPORT	SYSTEM_EnableVIC
SYSTEM_EnableVIC		PROC
						mrc 	p15,0,r0,c1,c0,0
						orr 	r0,r0,#(1<<24)
						mcr 	p15,0,r0,c1,c0,0
						mov		pc,lr
						ENDP


						EXPORT	SYSTEM_DisableVIC
SYSTEM_DisableVIC		PROC
						mrc 	p15,0,r0,c1,c0,0
						bic 	r0,r0,#(1<<24)
						mcr 	p15,0,r0,c1,c0,0
						mov		pc,lr
						ENDP


						EXPORT	SYSTEM_EnableIRQ
SYSTEM_EnableIRQ		PROC
						mrs		r0,cpsr
						bic		r0,r0,#(1<<7)
						msr		cpsr_cxsf,r0             
						mov		pc,lr
						ENDP


						EXPORT	SYSTEM_DisableIRQ				
SYSTEM_DisableIRQ		PROC
						mrs		r0,cpsr
						orr		r0,r0,#(1<<7)
						msr		cpsr_cxsf,r0             
						mov		pc,lr
						ENDP
				
				

						EXPORT	SYSTEM_EnableFIQ
SYSTEM_EnableFIQ		PROC
						mrs		r0,cpsr
						bic		r0,r0,#(1<<6)
						msr		cpsr_cxsf,r0             
						mov		pc,lr
						ENDP


						EXPORT	SYSTEM_DisableFIQ
SYSTEM_DisableFIQ		PROC
						mrs		r0,cpsr
						orr		r0,r0,#(1<<6)
						msr		cpsr_cxsf,r0             
						mov		pc,lr
						ENDP
;;
;;===============================================================================
			   EXPORT Stop_WFI_Test
Stop_WFI_Test		 PROC

					 ;stmfd	sp!, {r0-r13}
					 stmfd	sp!, {r0-r6}

					 ldr r5, =0x7E00FA1C		; Inform7
					 ldr r6, [r5]		 
					 mov r0, #0
					 ldr r1, =0x7E00F90C
					 
					 mov r3, #0
					 mov r4, #0

					 
LOOP_STOP					 
					 add r3, r3, #1
					 cmp r3, #0x2
					 blt Wait_BLK_PWR1
					 
					 
					 mcr p15,0,r0,c7,c10,5 ; Data Memory Barrier
					 mcr p15,0,r0,c7,c10,4 ; test, Data Sync. Barrier 
					 
   					 mcr p15,0,r0,c7,c0,4
   					   					 
   					 
Wait_BLK_PWR1
   					 ldr r2, [r1]
   					 ;cmp r2, #0x7F
   					 ;cmp r2, #0x51
   					 cmp r2, r6
   					 bne Wait_BLK_PWR1
   					 
   					 add r4, r4, #1
   					 cmp r4, #2
   					 blt LOOP_STOP
   								
   					 ldmfd sp!, {r0-r6}
   					 					 
   					 mov pc,lr
					 ENDP

			   EXPORT Stop_WFI_Test1
Stop_WFI_Test1		 PROC

					 ;stmfd	sp!, {r0-r13}
					 stmfd	sp!, {r0-r6}

					 ldr r5, =0x7E00FA1C		; Inform7
					 ldr r6, [r5]		 
					 mov r0, #0
					 ldr r1, =0x7E00F90C
					 
					 mov r3, #0
					 mov r4, #0

					 
LOOP_STOP1					 
					 add r3, r3, #1
					 cmp r3, #0x2
					 blt Wait_BLK_PWR2
					 
					 
					 ldr r1, =0x7E00F800
					 mov r0, #0x2
					 str r0, [r1]
					 
					 nop
					 nop
					 nop
					 nop
					 nop
					 nop
					 nop
					 nop
					 nop
					 nop
					 
					 ;mcr p15,0,r0,c7,c10,5 ; Data Memory Barrier
					 ;mcr p15,0,r0,c7,c10,4 ; test, Data Sync. Barrier 
					 
   					 ;mcr p15,0,r0,c7,c0,4
   					 
   					  b .  					 
   					 
Wait_BLK_PWR2
   					 nop;
   					 nop;
   					 nop;
   					 nop;
   					 nop;
   					 nop;
   					 nop; 					 
   					 add r4, r4, #1
   					 cmp r4, #2
   					 blt LOOP_STOP1
   								
   					 ;b .
   					 			
   					 ldmfd sp!, {r0-r6}
   					 					 
   					 mov pc,lr
					 ENDP




			   EXPORT MMU_WaitForInterrupt
MMU_WaitForInterrupt PROC
					 mov r0, #0
;					 ldr r1, =0x7E00F90C
					 
					 mcr p15,0,r0,c7,c10,5 ; Data Memory Barrier
					 mcr p15,0,r0,c7,c10,4 ; test, Data Sync. Barrier 
					 ;mcr p15,0,r0,c7,c5,4	; Flush Prefetch Buffer
					 
   					 mcr p15,0,r0,c7,c0,4
   					 
   					 ;for test
   					 ;nop
   					 ;nop
   					 ;nop
   					 ;nop
   					 ;nop
   					 ;nop
   					 ;nop
   					 ;nop
   					 ;nop
   					 ;nop
   					 ;bl SYSTEM_EnableIRQ
   					 
   					 ;ldr r1, =0x71200014
   					 ;ldr r2, =0xffffffff
   					 ;str r2, [r1]
   					 
   					 ;ldr r1, =0x71200F00
   					 ;ldr r2, =0x0
   					 ;str r2, [r1]
   					 
   					 
;Wait_BLK_PWR
;   					  ldr r2, [r1]
;   					  cmp r2, #0x7F
;  					  bne Wait_BLK_PWR
   					 
   										 
  					 mov pc,lr
 
					 ENDP
					 
;;===============================================================================
;;
;;	MMU/Cache
;;
					 
						EXPORT	SYSTEM_EnableBP
SYSTEM_EnableBP			PROC				
						mrc		p15,0,r0,c1,c0,0
						orr		r0,r0,#(1<<11)
						mcr		p15,0,r0,c1,c0,0
						mov		pc,lr
						ENDP
				
						EXPORT	SYSTEM_EnableICache
SYSTEM_EnableICache		PROC				
						mrc		p15,0,r0,c1,c0,0
						orr		r0,r0,#(1<<12)
						mcr		p15,0,r0,c1,c0,0
						mov		pc,lr
						ENDP


						EXPORT	SYSTEM_DisableICache
SYSTEM_DisableICache	PROC
						mrc		p15,0,r0,c1,c0,0
						bic		r0,r0,#(1<<12)
						mcr		p15,0,r0,c1,c0,0
						mov		pc,lr
						ENDP


						EXPORT	SYSTEM_EnableDCache
SYSTEM_EnableDCache		PROC
						mrc		p15,0,r0,c1,c0,0
						orr		r0,r0,#(1<<2)
						mcr		p15,0,r0,c1,c0,0
						mov		pc,lr
		   				ENDP


						EXPORT	SYSTEM_DisableDCache
SYSTEM_DisableDCache	PROC
						mrc		p15,0,r0,c1,c0,0
						bic		r0,r0,#(1<<2)
						mcr		p15,0,r0,c1,c0,0
						mov		pc,lr
						ENDP


						EXPORT	SYSTEM_InvalidateEntireICache
SYSTEM_InvalidateEntireICache	PROC
						mov		r0,#0
						mcr		p15,0,r0,c7,c5,0
						mov		pc,lr
						ENDP


						EXPORT	SYSTEM_InvalidateEntireDCache
SYSTEM_InvalidateEntireDCache	PROC
						mov		r0,#0
						mcr		p15,0,r0,c7,c6,0
						mov		pc,lr
						ENDP
				
				
						EXPORT	SYSTEM_InvalidateBothCache
SYSTEM_InvalidateBothCache	PROC
						mov		r0,#0
						mcr		p15,0,r0,c7,c7,0
						mov		pc,lr
						ENDP


						EXPORT	SYSTEM_CleanEntireDCache
SYSTEM_CleanEntireDCache	PROC
						mov		r0,#0
						mcr		p15,0,r0,c7,c10,0
						mov		pc,lr
						ENDP

				
						EXPORT	SYSTEM_CleanInvalidateEntireDCache
SYSTEM_CleanInvalidateEntireDCache	PROC
						mov		r0,#0
						mcr		p15,0,r0,c7,c14,0
						mov		pc,lr
						ENDP



						EXPORT	SYSTEM_EnableMMU
SYSTEM_EnableMMU		PROC
						mrc		p15,0,r0,c1,c0,0
						orr		r0,r0,#(1<<0)
						mcr		p15,0,r0,c1,c0,0
						mov		pc,lr
						ENDP


						EXPORT	SYSTEM_DisableMMU
SYSTEM_DisableMMU		PROC
						mrc		p15,0,r0,c1,c0,0
						bic		r0,r0,#(1<<0)
						mcr		p15,0,r0,c1,c0,0
						mov		pc,lr
						ENDP


						EXPORT	SYSTEM_InvalidateTLB
SYSTEM_InvalidateTLB	PROC
						mov		r0,#0x0     
						mcr		p15,0,r0,c8,c7,0
						mov		pc,lr
						ENDP


						EXPORT	SYSTEM_SetTTBase
SYSTEM_SetTTBase		PROC				;	r0=TTBase
						mcr		p15,0,r0,c2,c0,0
						mov		pc,lr
						ENDP


						EXPORT	SYSTEM_SetDomain
SYSTEM_SetDomain		PROC				;	r0=Domain
						mcr		p15,0,r0,c3,c0,0
						mov		pc,lr
						ENDP


						EXPORT	SYSTEM_SetFCSEPID
SYSTEM_SetFCSEPID		PROC        		;	r0= FCSEPID
						mcr		p15,0,r0,c13,c0,0
						mov		pc,lr
						ENDP
   

						EXPORT	SYSTEM_EnableAlignFault
SYSTEM_EnableAlignFault	PROC
						mrc		p15,0,r0,c1,c0,0
						orr		r0,r0,#(1<<1)
						mcr		p15,0,r0,c1,c0,0
						mov		pc,lr
		   				ENDP


						EXPORT	SYSTEM_DisableAlignFault
SYSTEM_DisableAlignFault	PROC
						mrc		p15,0,r0,c1,c0,0
						bic		r0,r0,#(1<<1)
						mcr		p15,0,r0,c1,c0,0
						mov		pc,lr
						ENDP


						EXPORT	SYSTEM_ReadDFSR
SYSTEM_ReadDFSR			PROC
						mrc		p15,0,r0,c5,c0,0
						mov		pc,lr
						ENDP
				

						EXPORT	SYSTEM_ReadIFSR
SYSTEM_ReadIFSR			PROC
						mrc		p15,0,r0,c5,c0,1
						mov		pc,lr
						ENDP


						EXPORT	SYSTEM_ReadFAR
SYSTEM_ReadFAR			PROC
						mrc		p15,0,r0,c6,c0,0
						mov		pc,lr
						ENDP   

						EXPORT	EBI_Test_Assem
EBI_Test_Assem			PROC

						ldr	r1, =0x70200010	; Dest Addr of NFCON
						ldr	r2, =0x55555555
						ldr	r3, =0xaaaaaaaa
						ldr	r4, =0xffffffff
						ldr	r5, =0x5a5a5a5a

						ldr	r8, =0x00005555	; DATA
						ldr	r9, =0x70200010	; NFCON DATA Register
						
						ldr	r6, =0x0000aaaa	; DATA
						ldr	r7, =0x7030192C	; CFCON Register

						;---------- WW --------------
						; Store to NFCON Data register
						str	r8,[r9]			; DATA -> NFCON Data Register
						; Store to CFCON Register
						str	r6, [r7]			; DATA -> CFCON Data Register

						nop
						nop

						;---------- WR --------------						
						; Store to NFCON Data register
						str	r8,[r9]			; DATA -> NFCON Data Register
						; Read from CFCON Register
						ldr	r0, [r7]			; CFCON Data Register -> Register

						nop
						nop

						;---------- BWW --------------												
						stm	r1,{r2-r5}			; DATA -> NFCON Data Register
						; Store to CFCON Register
						str	r6, [r7]			; DATA -> CFCON Data Register

						nop
						nop

						;---------- BWR --------------																		
						stm	r1,{r2-r5}			; DATA -> NFCON Data Register
						; Read from CFCON Register
						ldr	r0, [r7]			; CFCON Data Register -> Register

						nop
						nop
						ENDP   

						EXPORT	Set_Ata_Cmd_START
Set_Ata_Cmd_START		PROC
						ldr		r8, = 0x0
						ldr		r9, =0x1				; START command
						ldr		r10, =0x70301908		; ATA_COMMAND register
						swp		r8, r9, [r10]			; swap operation
						mov		pc,lr
						ENDP   

						EXPORT	Set_Ata_Cmd_STOP
Set_Ata_Cmd_STOP		PROC
						ldr		r8, = 0x0
						ldr		r9, =0x0				; STOP command
						ldr		r10, =0x70301908		; ATA_COMMAND register
						swp		r8, r9, [r10]			; swap operation
						mov		pc,lr
						ENDP   

						EXPORT	Set_Ata_Cmd_ABORT
Set_Ata_Cmd_ABORT		PROC
						ldr		r8, = 0x0
						ldr		r9, =0x2				; ABORT command
						ldr		r10, =0x70301908		; ATA_COMMAND register
						swp		r8, r9, [r10]			; swap operation
						mov		pc,lr
						ENDP   

						EXPORT	Set_Ata_Cmd_CONTINUE
Set_Ata_Cmd_CONTINUE	PROC
						ldr		r8, = 0x0
						ldr		r9, =0x3				; CONTINUE command
						ldr		r10, =0x70301908		; ATA_COMMAND register
						swp		r8, r9, [r10]			; swap operation
						mov		pc,lr
						ENDP   
												
						EXPORT	MEMCOPY_TEST
MEMCOPY_TEST			PROC
					    ldr	r0, =0x51000000
					    ldr r1, =0x52000000
					    ldr r2, =0x8000
					    ldr r3, =0
TCOPY					    
					    ldmia r0!, {r5-r12}
					    stmia r1!,  {r5-r12}		
					    add r3, r3, #1
					    cmp r3, r2
					    ;add r0, r0, #0x20
					    ;add r1, r1, #0x20
					    blt	TCOPY
						mov		pc,lr
						ENDP 
						
						EXPORT	MEMCOPY_TEST0
MEMCOPY_TEST0			PROC
					    ldr	r0, =0x41000000
					    ldr r1, =0x42000000
					    ldr r2, =0x8000
					    ldr r3, =0
TCOPY0					    
					    ldmia r0!, {r5-r12}
					    stmia r1!,  {r5-r12}		
					    add r3, r3, #1
					    cmp r3, r2
					    ;add r0, r0, #0x20
					    ;add r1, r1, #0x20
					    blt	TCOPY0
						mov		pc,lr
						ENDP 


						EXPORT	MEMCOPY8
MEMCOPY8				PROC
						stmfd	sp!, {r0-r12}
					    ;ldr r0, =0x51000000
					    ;ldr r1, =0x52000000
					    ;ldr r2, =0x8000
					    ldr r3, =0
TCOPY8					    
					    ldmia r0!, {r5-r12}
					    stmia r1!,  {r5-r12}		
					    add r3, r3, #1
					    cmp r3, r2
					    ;add r0, r0, #0x20
					    ;add r1, r1, #0x20
					    blt	TCOPY8
					    ldmfd	sp!, {r0-r12}
						mov		pc,lr
						ENDP 

						EXPORT	MEMWRITE4
MEMWRITE4				PROC
						stmfd	sp!, {r0-r8}
					    ldr r4, =0
TWRITE4			    			
							    
					  	mov r5, r2
						mov r6, r3	    
						mov r7, r2
						mov r8, r3
					    
					    stmia r0!, {r5-r8}
					    add r4, r4, #1
					    cmp r4, r1
					    ;add r0, r0, #0x20
					    ;add r1, r1, #0x20
					    blt	TWRITE4
					    ldmfd	sp!, {r0-r8}
						mov		pc,lr
						ENDP 						

						EXPORT	MODEM_BlockCopy
MODEM_BlockCopy			PROC

						;LDR     r0, =src                 ; r0 = pointer to source block
						;LDR     r1, =dst                 ; r1 = pointer to destination block
						;MOV     r2, #nWord           ; r2 = number of words to copy

					      ;MOV		sp, #0x400
					      ;PUSH		{r4-r11}
					      
					      STMDB	r13!,{r0-r2,r4-r11,lr}
					    mov		pc,lr
						ENDP 		
;;
;;===============================================================================

				END

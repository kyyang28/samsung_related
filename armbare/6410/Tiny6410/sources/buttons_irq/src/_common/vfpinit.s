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
;	File Name : vfpinit.s
;  
;	File Description : This file contains the sub routines related to 
;						the floating point initialization.
;
;	Author	: Haksoo,Kim
;	Dept. : AP Development Team
;	Created Date : 2006/11/08
;	Version : 0.1 
; 
;	History
;	- Created with ARM example codes(Haksoo,Kim 2006/11/08)
;  
;*************************************************************************************/

; These routines can be assembled --apcs /interwork.

        AREA vfpinit, CODE, READONLY

VFPEnable       EQU     0x40000000
CM_BASE         EQU     0x10000000
CM_INIT         EQU     0x24
CM_LOCK         EQU     0x14
CM_LOCK_VALUE   EQU     0xA05F
VFPTST          EQU     2_11:SHL:12

Mode_UNDEF      EQU     0x1B ; bit pattern for undefined mode

UNDEF_VECTOR    EQU     0x4 ; address of undefined instruction vector (hivecs not handled)

        GBLL ARCH_V6_OR_LATER   ; Create global variable
  if "6" <= {ARCHITECTURE} ; ok until architecture 10
ARCH_V6_OR_LATER SETL {TRUE}
  else
ARCH_V6_OR_LATER SETL {FALSE}
  endif

    IF :DEF: INIT_CM10rev0_VFP

        EXPORT  CM_VFP_enable

CM_VFP_enable FUNCTION
        ; enable Integrator Core Module to allow use of VFP
        MOV     r0, #CM_BASE          ; set r0 to base address of core module
                                                                  ; register space
        LDR     r2, =CM_LOCK_VALUE
        STR     r2,[r0,#CM_LOCK]      ; unlock CM_INIT
        LDR     r1, [r0,#CM_INIT]
        BIC     r1, r1,#VFPTST        ; clear bits 12 and 13
        STR     r1, [r0,#CM_INIT]     ; of CM_INIT
        MOV     r2,#0
        STR     r2,[r0,#CM_LOCK]      ; lock CM_INIT
        BX      LR
        ENDFUNC

        ENDIF

    IF :DEF: SETUP_UNDEF_STACK

        EXPORT  Setup_Undef_Stack

Setup_Undef_Stack FUNCTION
        ; Now set up a stack for undefined mode
        MRS     r0, CPSR                   ; get CPSR value
        MOV     r1, r0                     ; take a working copy
        ORR     r1,r1, #Mode_UNDEF         ; set mode bits for Undefined mode
        MSR     CPSR_c, r1                 ; change to undefined mode
      if {RWPI}
        LDR     r2, UNDEF_Stack_Offset
        ADD     SP, r2, r9
      else
        LDR     SP, =UNDEF_Stack           ; set up the stack pointer
      endif
        MSR     CPSR_c, r0                 ; change back to the original mode
        BX      LR                         ; return from subroutine
        ENDFUNC
  
      if {RWPI}
UNDEF_Stack_Offset DCDO UNDEF_Stack
      endif

        ENDIF

    IF :DEF: PATCH_UNDEF_VECTOR

        EXPORT  Install_VFPHandler
        IMPORT  TLUndef_Handler

Install_VFPHandler FUNCTION
        ; Install VFP handler onto undefined instruction

; TLUndef_Handler must be reachable via BL from UNDEF_VECTOR.
        ADR     r0, TLUndef_Handler_Offset
        LDR     r1, [r0]
        ADD     r0, r0, r1
        SUB     r0, r0, #UNDEF_VECTOR+8 ; allow for vector address and PC offset
        MOV     r0, r0, LSR #2
        ORR     r0, r0, #0xea000000     ; bit pattern for Branch always
        MOV     r1, #UNDEF_VECTOR
  if ARCH_V6_OR_LATER :LAND: {ENDIAN} = "big" :LAND: :LNOT: :DEF: ENDIAN_BE_32
        REV     r0, r0
  endif
        STR     r0, [r1]

; If we have separate data and instruction caches then we need to clean the
; data cache and invalidate the instruction cache.
; If we have a branch target cache we need to invalidate that as well.

  if :DEF: WANT_CACHE_FLUSH

; This code is known to work with ARM926, ARM946, ARM1020, ARM1022, ARM1026 and ARM1136 cores.
; For other cores the following may need to be modified.  Please check the TRM for your core.

CACHE_ADDR_SBZ EQU 0x7  ; these bits "should be zero" when cleaning/invalidating by virtual address
                        ; v6 doesn't have SBZ bits
UNDEF_VECTOR_CACHE_LINE EQU (UNDEF_VECTOR :AND: :NOT:CACHE_ADDR_SBZ)
        mov r1, #UNDEF_VECTOR_CACHE_LINE

        mcr p15, 0, r1, c7, c10, 1 ; clean D cache at 'r1'
        mcr p15, 0, r1, c7, c5,  1 ; invalidate I cache at 'r1'
        mov r0,#0
        mcr p15, 0, r0, c7, c10, 4 ; drain write buffer

    if ARCH_V6_OR_LATER
        mov r1, #UNDEF_VECTOR
        mcr p15, 0, r1, c7, c5,  7 ; invalidate branch target cache at 'r1'
    endif

  endif

        BX      LR                              ; return from subroutine
TLUndef_Handler_Offset
        DCD     TLUndef_Handler - TLUndef_Handler_Offset
        ENDFUNC

        ENDIF

        EXPORT  Enable_VFP

Enable_VFP FUNCTION

  if ARCH_V6_OR_LATER
        MRC p15, 0, r1, c1, c0, 2 ; r1 = Access Control Register
        ORR r1, r1, #(0xf << 20)    ; enable full access for p10,11
        MCR p15, 0, r1, c1, c0, 2 ; Access Control Register = r1
        MOV r1, #0
        MCR p15, 0, r1, c7, c5, 4 ; flush prefetch buffer because of FMXR below and
                                  ; CP 10 & 11 were only just enabled
  endif

        ; Enable VFP itself
        MOV     r0,#VFPEnable
        FMXR    FPEXC, r0       ; FPEXC = r0
        BX      LR
        ENDFUNC

    IF :DEF: SETUP_UNDEF_STACK

; Location for undefined-mode stack

        AREA   UNDEF_STACK, NOINIT, ALIGN=3
        %       8               ; Only two words will be used
UNDEF_Stack   EQU .

        ENDIF

        END

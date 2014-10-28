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
*	File Name : fpinit.c
*  
*	File Description : This file contains the floating point initialization.
*
*	Author : Haksoo,Kim
*	Dept. : AP Development Team
*	Created Date : 2006/11/08
*	Version : 0.1 
* 
*	History
*	- Created with ARM example codes(Haksoo,Kim 2006/11/08)
*  
**************************************************************************************/

#ifdef __cplusplus
 #define EXTERN_C extern "C"
#else
 #define EXTERN_C extern
#endif

#ifdef SETUP_UNDEF_STACK
EXTERN_C void Setup_Undef_Stack (void);
#endif
#ifdef PATCH_UNDEF_VECTOR
EXTERN_C void Install_VFPHandler (void);
#endif

#ifdef INIT_CM10rev0_VFP
EXTERN_C void CM_VFP_enable (void);
#endif

EXTERN_C void Enable_VFP (void);

EXTERN_C void $Super$$_fp_init(void);

EXTERN_C void $Sub$$_fp_init(void)   // Function called in place of original _fp_init()
{

#ifdef PATCH_UNDEF_VECTOR
        Install_VFPHandler();
#endif
#ifdef SETUP_UNDEF_STACK
        Setup_Undef_Stack();
#endif
#ifdef INIT_CM10rev0_VFP
        CM_VFP_enable();
#endif
        Enable_VFP();
        $Super$$_fp_init();   // Call original _fp_init()
}


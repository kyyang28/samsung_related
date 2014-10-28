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
;	File Name : startup.s
;  
;	File Description : This file implements the startup procedure.
;
;	Author	: Heemyung.Noh
;	Dept. : AP Development Team
;	Created Date : 2008/02/20
;	Version : 0.1 
; 
;	History
;	- Created for Scatterloading&TCM option(Heemyung.Noh 2008/02/20)
;  
;*************************************************************************************/

				GET		option.inc

;=======================================================
;						ENTRY  
;=======================================================

				AREA    Init,CODE,READONLY

				ENTRY 
ResetHandler
				
				LDR R13, =top_of_stacks
				
				EXPORT	__main
__main
				;; initialise the C library (which calls main())
				IMPORT	__rt_entry
				LDR	R0, =__rt_entry
				BX   	R0
				
			   	b		.
;=======================================================			   	
		        END



.globl _start
_start:
	/* Peripheral port setup */
	ldr r0, =0x70000000
	orr r0, r0, #0x13
	mcr p15,0,r0,c15,c2,4
	
	/*	
	 *	OPTIONS: During BL0, the watchdog is already disabled, 
	 *	so actually we don't need to disable it again(in BL1)
	 */
	ldr	r0, =0x7E004000
	mov	r1, #0
	str r1, [r0]
	
	/*	
	 *	WARNING: 
	 *	1.	When booting from SD card, DO NOT setup the stack here(in BL1) 
	 *		since the stack is already setup during BL0,
	 *		otherwise, the program may run abnormally
	 *	2.	When booting from Nandflash, DO remember to setup the stack.
	 */
#ifdef BOOT_FROM_NAND
	ldr sp, =8*1024
#endif
	
	/* Jump to myled */
	bl	myled
		
_infinite_loop:
	b	_infinite_loop


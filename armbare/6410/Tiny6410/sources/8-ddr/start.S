
.globl _start
_start:
	b	start_code

.globl _bss_start
_bss_start:
	.word __bss_start

.globl _bss_end
_bss_end:
	.word __bss_end

start_code:
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
	 *	2.	When booting from Nandflash, DO setup the stack
	 */
#ifdef BOOT_FROM_NAND
	ldr sp, =8*1024
#endif

	/* Clock init */
	bl	clock_init

	/* DDR init */
	bl	ddr_init

relocate:
	adr	r0, _start
	ldr r1, =_start
	ldr r2, _bss_start
	cmp r0, r1
	beq	clear_bss

copy_loop:
	ldr r3, [r0], #4
	str r3, [r1], #4
	cmp r1, r2
	bne	copy_loop
	
	/* Clear BSS Section */	
clear_bss:
	ldr r0, _bss_start
	ldr r1, _bss_end
	cmp r0, r1
	beq	_myddr
	mov	r2, #0
	
clbss_l:
	str	r2, [r0], #4
	cmp r0, r1
	bne	clbss_l
	
_myddr:
	/* Jump to myddr */
	ldr pc, __myddr

__myddr:
	.word myddr

_infinite_loop:
	b	_infinite_loop



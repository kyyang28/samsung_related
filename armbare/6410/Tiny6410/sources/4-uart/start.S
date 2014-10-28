
.text
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
	
	/* DO NOT setup the stack if booting from SDCARD */
#ifdef BOOT_FROM_NAND
	ldr sp, =8*1024
#endif

	bl	clock_init
	@bl  led_init
	bl	uart_init

	/* Jump to uart */
	bl	myuart
	
_infinite_loop:
	b	_infinite_loop



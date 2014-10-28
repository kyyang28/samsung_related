
.globl _start
_start:
	/* 
	 *	iROM BL1 has already disabled watchdog, so that
	 *	acturally, we don't need to disable it again
	 */

	/* Clock init */
	bl	clock_asm_init

	/* Setup the stack on SRAM */
	ldr	sp, =0xD0030000

	/* Jump to clock_leds_main */
	ldr	pc, =clock_leds_main

_infinite_loop:
	b	_infinite_loop


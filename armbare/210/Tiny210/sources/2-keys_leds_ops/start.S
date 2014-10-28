
.globl _start
_start:
	/* 
	 *	iROM BL1 has already disabled watchdog, so that
	 *	acturally, we don't need to disable it again
	 */
	
	/* Setup the stack on SRAM */
	ldr	sp, =0xD0030000

	/* Jump to keys_leds_main */
	ldr	pc,	=keys_leds_main

_infinite_loop:
	b	_infinite_loop


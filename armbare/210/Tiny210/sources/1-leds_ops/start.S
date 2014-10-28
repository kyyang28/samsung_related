
.globl _start
_start:
	/* 
	 *	iROM BL1 has already disabled watchdog, so that
	 *	acturally, we don't need to disable it again
	 */
	
	/* 
	 *	Setup the stack on internal SRAM,
	 *	referenced by S5PV210_iROM_ApplicationNote_Preliminary_20091126.pdf
	 */
	ldr	sp, =0xD0030000

	/* Jump to leds_main */
	ldr	pc, =leds_main

_infinite_loop:
	b	_infinite_loop


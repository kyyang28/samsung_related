
.globl _start
_start:
	/* 
	 *	iROM BL1 has already disabled watchdog, so that
	 *	acturally, we don't need to disable it again
	 */

	/* Clock init */
	bl	clock_asm_init	

	/* Setup the stack on iSRAM */
	ldr	sp, =0xD0030000

	/* Jump to clock_uart_main */
	ldr	pc, =clock_uart_main

_infinite_loop:
	b	_infinite_loop


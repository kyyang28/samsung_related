
.globl _start
_start:

	/* Disable watchdog */
	ldr	r0, =0x53000000
	mov r1, #0x0
	str r1, [r0]

	/* Setup the stack */
	ldr	sp, =4096

	/* Jump to leds_main function which is implemented by C */
	bl	leds_main

infinite_loop:
	b	infinite_loop


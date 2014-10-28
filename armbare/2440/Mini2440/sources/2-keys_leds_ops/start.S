
.globl _start
_start:
	
	/* Disable watchdog */
	ldr	r0, =0x53000000
	mov	r1, #0x0
	str r1, [r0]
	
	/* Setup the stack */
	ldr	sp, =4096

	/* Jump to keys_leds_main */
	bl	keys_leds_main

infinite_loop:
	b	infinite_loop


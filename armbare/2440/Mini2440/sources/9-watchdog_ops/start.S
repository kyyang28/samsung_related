

.globl _start
_start:
	
#define WT_BASE_ADDR	0x53000000

#define WTCON_OFS		0x00
#define WTDAT_OFS		0x04
#define WTCNT_OFS		0x08

#define WTCON_VALS		(74<<8 | 0x01<<5 | 0x01)
#define WTCNT_VALS		(20000)
	/* Setup the watchdog timer registers */
	ldr r0, =WT_BASE_ADDR

	/* Setup the WTCON register */
	ldr r1,	=WTCON_VALS
	str r1, [r0, #WTCON_OFS]

	/* Setup the WTCNT register */
	ldr r1, =WTCNT_VALS
	str r1, [r0, #WTCNT_OFS]

	/* Setup the stack */
	ldr	sp, =4096

	/* Jump to watchdog_main */
	ldr	pc, =watchdog_main

infinite_loop:
	b	infinite_loop


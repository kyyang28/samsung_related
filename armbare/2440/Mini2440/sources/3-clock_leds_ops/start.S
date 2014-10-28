

#define LOCKTIME					(0x4C000000)
#define MPLLCON						(0x4C000004)
#define CLKDIVN						(0x4C000014)
#define CAMDIVN						(0x4C000018)

/* 
 *	HDIVN [2:1] = 10 (HCLK = FCLK/4 when CAMDIVN[9] = 0)
 *	PDIVN [0] = 1 (PCLK has the clock same as the HCLK/2)
 */
#define CLKDIVN_VAL					(0x05)
#define MINI2440_MPLL_405MHZ		((0x7F<<12) | (0x2<<4) | (0x1))

.globl _start
_start:
	/* Disable watchdog */
	ldr	r0, =0x53000000
	mov	r1, #0x0
	str r1, [r0]

	/* @@@@ Start of clock initialization @@@@ */
	/* Setup the locktime value */
	ldr	r0, =LOCKTIME
	mov	r1, #0xFFFFFFFF
	str r1, [r0]

	/* Set the cpu bus mode to asynchronous */
	mrc p15, 0, r0, c1, c0, 0
	orr r0, r0, #0xC0000000
	mcr	p15, 0, r0, c1, c0, 0

	/* Setup the clock divide value */
	ldr	r0, =CLKDIVN
	mov	r1, #CLKDIVN_VAL
	str	r1, [r0]

	/* Setup the MPLL value */
	ldr r0, =MPLLCON
	ldr r1, =MINI2440_MPLL_405MHZ
	str r1, [r0]

	/* @@@@ End of clock initialization @@@@ */
	
	/* Setup the stack */
	ldr	sp, =4096

	/* Jump to clock_led_main */
	bl	clock_led_main

infinite_loop:
	b	infinite_loop


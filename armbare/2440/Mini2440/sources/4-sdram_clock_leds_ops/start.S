

/* @@@@@@@@ Start of clock registers and setting values @@@@@@@@ */

#define CLOCK_BASE_ADDR				(0x4C000000)
#define LOCKTIME_OFS				(0x00)
#define MPLLCON_OFS					(0x04)
#define CLKDIVN_OFS					(0x14)

/* 
 *	HDIVN [2:1] = 10 (HCLK = FCLK/4 when CAMDIVN[9] = 0)
 *	PDIVN [0] = 1 (PCLK has the clock same as the HCLK/2)
 */
#define CLKDIVN_VAL					(0x05)
#define MINI2440_MPLL_405MHZ		((0x7F<<12) | (0x2<<4) | (0x1))
/* @@@@@@@@ End of clock registers and setting values @@@@@@@@ */

.globl _start
_start:
	/* Disable watchdog */
	ldr	r0, =0x53000000
	mov	r1, #0x0
	str r1, [r0]

	/* @@@@ Start of clock initialization @@@@ */
	/* Setup the locktime value */
	ldr	r0, =CLOCK_BASE_ADDR
	mov	r1, #0xFFFFFFFF
	str r1, [r0, #LOCKTIME_OFS]

	/* Set the cpu bus mode to asynchronous */
	mrc p15, 0, r1, c1, c0, 0
	orr r1, r1, #0xC0000000
	mcr	p15, 0, r1, c1, c0, 0

	/* Setup the clock divide value */
	mov	r1, #CLKDIVN_VAL
	str	r1, [r0, #CLKDIVN_OFS]

	/* Setup the MPLL value */
	ldr r1, =MINI2440_MPLL_405MHZ
	str r1, [r0, #MPLLCON_OFS]

	/* @@@@ End of clock initialization @@@@ */
	
	/* No need to setup the stack, since we use asm codes on SRAM */
	@ldr	sp, =4096

	bl	sdram_asm_init

	/* @@@@ Start: copy code from SRAM to SDRAM @@@@ */
	mov	r0, #0
	ldr r1, =0x30000000
	ldr r2, =4096

copy_loop:
	ldr r3, [r0], #4
	str r3, [r1], #4
	cmp r0, r2
	bne copy_loop
	/* @@@@ End: copy code from SRAM to SDRAM @@@@ */

	ldr	pc, =jump2sdram

jump2sdram:
	
	ldr sp, =0x34000000

	/* Jump to clock_led_main */
	bl	sdram_clk_leds_main	

infinite_loop:
	b	infinite_loop


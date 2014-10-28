
#define CLOCK_BASE_ADDR						(0x4C000000)

#define LOCKTIME_OFS						(0x00)
#define MPLLCON_OFS							(0x04)
#define CLKDIVN_OFS							(0x14)

#define	CLKDIVN_VALS						(0x05)	/* 1:4:8 */
#define MINI2440_MPLLCON_405MHZ				((0x7F<<12) | (0x2<<4) | (0x1))

.globl clock_asm_init
clock_asm_init:

	/* Load the clock base address */
	ldr	r0, =CLOCK_BASE_ADDR

	/* Setup the LOCKTIME register */
	ldr	r1, =0xFFFFFFFF
	str r1, [r0, #LOCKTIME_OFS]

	/* Switch to asynchronous cpu bus mode */
	mrc p15, 0, r1, c1, c0, 0
	orr r1, r1, #0xC0000000
	mcr p15, 0, r1, c1, c0, 0

	/* Setup the CLKDIVN register, change to 1:4:8 */
	mov	r1, #CLKDIVN_VALS
	str r1, [r0, #CLKDIVN_OFS]

	/* Setup the MPLLCON register to 405MHZ */
	ldr r1, =MINI2440_MPLLCON_405MHZ
	str r1, [r0, #MPLLCON_OFS]

	mov	pc, lr


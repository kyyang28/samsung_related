

#define MEM_CONTROLLER_BASE_ADDR		(0x48000000)

#define BWSCON_OFS						(0x00)
#define BANKCON6_OFS					(0x1C)
#define REFRESH_OFS						(0x24)
#define BANKSIZE_OFS					(0x28)
#define MRSRB6_OFS						(0x2C)

#define BWSCON_VALS						(0x2<<24)
#define BANKCON6_VALS					(0x3<<15 | 0x1<<2 | 0x1)
#define REFRESH_VALS					(0x1<<23 | 0x0<<22 | 0x0<<20 | 0x3<<18 | 0x4F4)
#define BANKSIZE_VALS					(0x1<<7 | 0x1<<5 | 0x1<<4 | 0x1)
#define MRSRB6_VALS						(0x3<<4)

.globl sdram_asm_init
sdram_asm_init:
	ldr r0, =MEM_CONTROLLER_BASE_ADDR
	
	/* Setup the BWSCON register */
	ldr r1, =BWSCON_VALS
	str r1, [r0, #BWSCON_OFS]

	/* Setup the BANKCON6 register */
	ldr r1, =BANKCON6_VALS
	str r1, [r0, #BANKCON6_OFS]

	/* Setup the REFRESH register */
	ldr r1, =REFRESH_VALS
	str r1, [r0, #REFRESH_OFS]

	/* Setup the BANKSIZE register */
	ldr r1, =BANKSIZE_VALS
	str r1, [r0, #BANKSIZE_OFS]

	/* Setup the MRSRB6 register */
	ldr r1, =MRSRB6_VALS
	str r1, [r0, #MRSRB6_OFS]

	mov pc, lr




#include "s3c2440-regs.h"

.globl sdram_asm_init
sdram_asm_init:
	
	/* Load the memory controller base address */
	ldr r0, =MEMORY_CONTROLLER_BASE_ADDR

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

	mov	pc, lr


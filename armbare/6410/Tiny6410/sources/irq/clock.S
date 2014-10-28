.globl clock_init

clock_init:
	
	/* 1.设置LOCK_TIME */
	ldr r0, =0x7E00F000  /* APLL_LOCK */
	ldr r1, =0x0000FFFF
	str r1, [r0]
	
	str r1, [r0, #4]	 /* MPLL_LOCK */
	str r1, [r0, #8]	 /* EPLL_LOCK */	
	
#define OTHERS		0x7e00f900
	@ set async mode  /* 当CPU时钟 != HCLK时，要设为异步模式 */
	ldr r0, =OTHERS
	ldr r1, [r0]
	bic r1, #0xc0			
	str r1, [r0]

loop1:				/* 等待，直到CPU进入异步模式 */
	ldr r0, =OTHERS
	ldr r1, [r0]
	and r1, #0xf00					
	cmp r1, #0
	bne loop1		
	
	/* SYNC667 */
	/* MISC_CON[19] = 0 */

#define ARM_RATIO    0   /* ARMCLK = DOUTAPLL / (ARM_RATIO + 1)    */
#define HCLKX2_RATIO 1   /* HCLKX2 = HCLKX2IN / (HCLKX2_RATIO + 1) */
#define HCLK_RATIO   1   /* HCLK = HCLKX2 / (HCLK_RATIO + 1)       */
#define PCLK_RATIO   3   /* PCLK   = HCLKX2 / (PCLK_RATIO + 1)     */
#define MPLL_RATIO   0   /* DOUTMPLL = MOUTMPLL / (MPLL_RATIO + 1)     */
	ldr r0, =0x7E00F020  /* CLK_DIV0 */
	ldr r1, =(ARM_RATIO) | (MPLL_RATIO << 4) | (HCLK_RATIO << 8) | (HCLKX2_RATIO << 9) | (PCLK_RATIO << 12)
	str r1, [r0]
	
	/* 2.配置时钟 */
	/* 2.1 配置APLL */
	/* 2.1.1 设置APLL
	 * 2.1.2 MUXAPLL
	 * 2.1.3 SYNC667
	 * 2.1.4 DIVAPLL
	 */
#define APLL_CON_VAL  ((1<<31) | (266 << 16) | (3 << 8) | (1))
	ldr r0, =0x7E00F00C
	ldr r1, =APLL_CON_VAL
	str r1, [r0]		/* APLL_CON, FOUTAPL = MDIV * Fin / (PDIV*2^SDIV) = 266*12/(3*2^1) = 532MHz  */
	
	/* 2.2 配置MPLL */
	/* 2.2.1 设置MPLL
	 * 2.2.2 MUXMPLL
	 * 2.2.3 SYNCMUX
	 * 2.2.4 SYNC667
	 * 2.2.5 HCLKX2_RATIO
	 * 2.2.6 PCLK_RATIO
	 */
#define MPLL_CON_VAL  ((1<<31) | (266 << 16) | (3 << 8) | (1))
	ldr r0, =0x7E00F010
	ldr r1, =MPLL_CON_VAL
	str r1, [r0]		/* MPLL_CON, FOUTMPL = MDIV * Fin / (PDIV*2^SDIV) = 266*12/(3*2^1) = 532MHz  */
	
	/* 3.选择PLL的输出作为时钟源 */
	ldr r0, =0x7E00F01C
	ldr r1, =0x03
	str r1, [r0]
	
	mov pc, lr


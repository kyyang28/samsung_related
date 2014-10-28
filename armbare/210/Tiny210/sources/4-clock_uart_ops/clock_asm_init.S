

/* #################### Clock Base Address #################### */
#define CLOCK_BASE_ADDR				(0xE0100000)

/* #################### Offset #################### */
/* Lock */
#define APLL_LOCK_OFS				(0x000)
#define MPLL_LOCK_OFS				(0x008)
#define EPLL_LOCK_OFS				(0x010)
#define VPLL_LOCK_OFS				(0x020)

/* Control */
#define APLL_CON0_OFS				(0x100)
#define APLL_CON1_OFS				(0x104)
#define MPLL_CON_OFS				(0x108)
#define EPLL_CON0_OFS				(0x110)
#define EPLL_CON1_OFS				(0x114)
#define VPLL_CON_OFS				(0x120)

/* Source */
#define CLK_SRC0_OFS				(0x200)

/* Divider */
#define CLK_DIV0_OFS				(0x300)


/* ################################# Values ################################# */
/* LOCK values */
#define LOCK_VAL					(0x0FFF)

/* APLL_CON0 values */
#define APLL_DISABLE				(0x00)
#define APLL_ENABLE					(0x01)
#define APLL_CON0_MDIV				(0x7D)
#define APLL_CON0_PDIV				(0x03)
#define APLL_CON0_SDIV				(0x01)
#define APLL_CON0_VAL				((APLL_ENABLE<<31) | (APLL_CON0_MDIV<<16) \
									 | (APLL_CON0_PDIV<<8) | (APLL_CON0_SDIV))	/* APLLCLK = 1GHz */

/* MPLL_CON0 values */
#define MPLL_DISABLE				(0x00)
#define MPLL_ENABLE					(0x01)
#define MPLL_CON_MDIV				(0x29B)
#define MPLL_CON_PDIV				(0x00C)
#define MPLL_CON_SDIV				(0x001)
#define MPLL_CON_VAL				((MPLL_ENABLE<<31) | (MPLL_CON_MDIV<<16) \
									 | (MPLL_CON_PDIV<<8) | (MPLL_CON_SDIV))	/* MPLLCLK = 667MHz */

/* CLK_SRC0 values */
#define MUXAPLL_FINPLL				(0x0)
#define MUXAPLL_FOUTAPLL			(0x1)
#define MUXMPLL_FINPLL				(0x0)
#define MUXMPLL_FOUTMPLL			(0x1)
#define MUXEPLL_FINPLL				(0x0)
#define MUXEPLL_FOUTEPLL			(0x1)
#define MUXVPLL_FINVPLL				(0x0)
#define MUXVPLL_FOUTVPLL			(0x1)
#define MUX_MSYS_SCLKAPLL			(0x0)
#define MUX_MSYS_SCLKMPLL			(0x1)
#define MUX_DSYS_SCLKMPLL			(0x0)
#define MUX_DSYS_SCLKA2M			(0x1)
#define MUX_PSYS_SCLKMPLL			(0x0)
#define MUX_PSYS_SCLKA2M			(0x1)
#define MUXFLASH_HCLK_PSYS			(0x0)
#define MUXFLASH_HCLK_DSYS			(0x1)
#define CLK_SRC0_VAL				((MUXVPLL_FOUTVPLL<<12) | (MUXEPLL_FOUTEPLL<<8) \
									 | (MUXMPLL_FOUTMPLL<<4) | (MUXAPLL_FOUTAPLL))

/* CLK_DIV0 values */
/* MOUT_MSYS = 1GHz */
#define APLL_RATIO					(0x0)	/* ARMCLK = SCLKAPLL = MOUT_MSYS / (APLL_RATIO + 1)  */
#define A2M_RATIO					(0x2)	/* SCLKA2M = SCLKAPLL / (A2M_RATIO + 1) */
#define HCLK_MSYS_RATIO				(0x4)	/* HCLK_MSYS = ARMCLK / (HCLK_MSYS_RATIO + 1) */
#define PCLK_MSYS_RATIO				(0x1)	/* PCLK_MSYS = HCLK_MSYS / (PCLK_MSYS_RATIO + 1) */
#define HCLK_DSYS_RATIO				(0x3)	/* HCLK_DSYS = MOUT_DSYS / (HCLK_DSYS_RATIO + 1) */
#define PCLK_DSYS_RATIO				(0x1)	/* PCLK_DSYS = HCLK_DSYS / (PCLK_DSYS_RATIO + 1) */
#define HCLK_PSYS_RATIO				(0x4)	/* HCLK_PSYS = MOUT_PSYS / (HCLK_PSYS_RATIO + 1) */
#define PCLK_PSYS_RATIO				(0x1)	/* PCLK_PSYS = HCLK_PSYS / (PCLK_PSYS_RATIO + 1) */

#define CLK_DIV0_VAL				((PCLK_PSYS_RATIO<<28) | (HCLK_PSYS_RATIO<<24) \
		                             | (PCLK_DSYS_RATIO<<20) | (HCLK_DSYS_RATIO<<16) \
		                             | (PCLK_MSYS_RATIO<<12) | (HCLK_MSYS_RATIO<<8) \
									 | APLL_RATIO)

.globl clock_asm_init
clock_asm_init:
	/* Load the CLOCK_BASE_ADDR to r0 */
	ldr	r0, =CLOCK_BASE_ADDR

	/* Setup the APLL_LOCK & MPLL_LOCK Register */
	ldr r1, =LOCK_VAL
	str r1, [r0, #APLL_LOCK_OFS]
	str r1, [r0, #MPLL_LOCK_OFS]

	/* Setup the APLL_CON0 Register */
	ldr	r1, =APLL_CON0_VAL
	str r1, [r0, #APLL_CON0_OFS]

	/* Setup the MPLL_CON0 Register */
	ldr	r1, =MPLL_CON_VAL
	str r1, [r0, #MPLL_CON_OFS]

	/* Setup the CLK_SRC0 Register */
	ldr	r1, =CLK_SRC0_VAL
	str r1, [r0, #CLK_SRC0_OFS]
	
	/* Setup the CLK_DIV0 Register */
	ldr	r1, =CLK_DIV0_VAL
	str r1, [r0, #CLK_DIV0_OFS]
	
	mov pc, lr
	

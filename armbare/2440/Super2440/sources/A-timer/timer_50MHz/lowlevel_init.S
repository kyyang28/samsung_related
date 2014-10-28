
/* BWSCON */
#define DW8		 	(0x0)
#define DW16		 	(0x1)
#define DW32		 	(0x2)

#define B1_BWSCON	  	(DW16)
#define B2_BWSCON	  	(DW16)
#define B3_BWSCON	  	(DW16)
#define B4_BWSCON	  	(DW16)
#define B5_BWSCON	  	(DW8)
#define B6_BWSCON	  	(DW32)
#define B7_BWSCON	  	(DW32)

/* BANKCON0 */
#define B0_Tacs			(0x0) /* 0 clock */
#define B0_Tcos			(0x0) /* 0 clock */
#define B0_Tacc			(0x7) /* 14 clock */
#define B0_Tcoh			(0x0) /* 0 clock */
#define B0_Tcah			(0x0) /* 0 clock */
#define B0_Tacp			(0x0) /* 2 clock */
#define B0_PMC			(0x0) /* normal */

/* BANKCON1 */
#define B1_Tacs			(0x0) /* 0 clock */
#define B1_Tcos			(0x0) /* 0 clock */
#define B1_Tacc			(0x7) /* 14 clock */
#define B1_Tcoh			(0x0) /* 0 clock */
#define B1_Tcah			(0x0) /* 0 clock */
#define B1_Tacp			(0x0) /* 2 clock */
#define B1_PMC			(0x0) /* normal */

/* BANKCON2 */
#define B2_Tacs			(0x0) /* 0 clock */
#define B2_Tcos			(0x0) /* 0 clock */
#define B2_Tacc			(0x7) /* 14 clock */
#define B2_Tcoh			(0x0) /* 0 clock */
#define B2_Tcah			(0x0) /* 0 clock */
#define B2_Tacp			(0x0) /* 2 clock */
#define B2_PMC			(0x0) /* normal */

/* BANKCON3 */
#define B3_Tacs			(0x0) /* 0 clock */
#define B3_Tcos			(0x0) /* 0 clock */
#define B3_Tacc			(0x7) /* 14 clock */
#define B3_Tcoh			(0x0) /* 0 clock */
#define B3_Tcah			(0x0) /* 0 clock */
#define B3_Tacp			(0x0) /* 2 clock */
#define B3_PMC			(0x0) /* normal */

/* BANKCON4 */
#define B4_Tacs			(0x0) /* 0 clock */
#define B4_Tcos			(0x0) /* 0 clock */
#define B4_Tacc			(0x7) /* 14 clock */
#define B4_Tcoh			(0x0) /* 0 clock */
#define B4_Tcah			(0x0) /* 0 clock */
#define B4_Tacp			(0x0) /* 2 clock */
#define B4_PMC			(0x0) /* normal */

/* BANKCON5 */
#define B5_Tacs			(0x0) /* 0 clock */
#define B5_Tcos			(0x0) /* 0 clock */
#define B5_Tacc			(0x7) /* 14 clock */
#define B5_Tcoh			(0x0) /* 0 clock */
#define B5_Tcah			(0x0) /* 0 clock */
#define B5_Tacp			(0x0) /* 2 clock */
#define B5_PMC			(0x0) /* normal */

/* BANKCON6 */
#define B6_MT			(0x3) /* SDRAM */
#define B6_Trcd			(0x1) /* 3 clocks */
#define B6_SCAN			(0x1) /* Column address 9-bit */

/* BANKCON7 */
#define B7_MT			(0x3) /* SDRAM */
#define B7_Trcd			(0x1) /* 3 clocks */
#define B7_SCAN			(0x1) /* Column address 9-bit */

/* REFRESH */
#define REFEN			(0x1) /* Refresh enable */
#define TREFMD			(0x0) /* CBR/Auto Refresh */
#define Trp			(0x0) /* SDRAM RAS pre-charge Time - 2 clocks */
#define Tsrc			(0x3) /* SDRAM Semi Row cycle time - 7 clocks */
#define REFCNT			(0x4F4) 
				/* Refresh period = (2^11-refresh_count+1)/HCLK */
				/* 8192 refresh cycles / 64ms = 7.8125 us
				   If HCLK = 12MHz: 
				      REFCNT = 2^11 + 1 - 7.8125*12 = 1955 = 0x7A3  
				   If HCLK = 100MHz:
				      REFCNT = 2^11 + 1 - 7.8125*100 = 1268 = 0x4F4 
				 */

/* BANKSIZE */
#define BURST_EN		(0x1) /* Enable burst operation */
#define SCKE_EN			(0x1) /* SDRAM power down mode enable */
#define SCLK_EN			(0x1) /* SCLK is active only during the access */
#define BK76MAP			(0x1) /* 64MB/64MB */

/* MRSRB6 */
#define WBL			(0x0) /* Write burst length - Burst (Fixed) */
#define TM			(0x0) /* Test mode - Mode register set (Fixed) */
#define CL			(0x3) /* CAS latency - 3 clocks */
#define	BT			(0x0) /* Burst type - Sequential (Fixed) */
#define BL			(0x0) /* Burst length - 1 (Fixed) */

/* MRSRB7 */
#define WBL			(0x0) /* Write burst length - Burst (Fixed) */
#define TM			(0x0) /* Test mode - Mode register set (Fixed) */
#define CL			(0x3) /* CAS latency - 3 clocks */
#define	BT			(0x0) /* Burst type - Sequential (Fixed) */
#define BL			(0x0) /* Burst length - 1 (Fixed) */


.equ	  SDRAM_MEM_BASE,	0x48000000 

.text
.global sdram_init
sdram_init:
    	mov 	r1,     #SDRAM_MEM_BASE       
	adrl	r2, 	MEM_INIT_VAL
	add 	r3,     r1, #4*13             
1:  
    	ldr 	r4,     [r2], #4            
    	str 	r4,     [r1], #4            
    	cmp 	r1,     r3                  
    	bne 	1b                          
    	mov 	pc,     lr                  

 
.ltorg
/* the literal pools origin */

MEM_INIT_VAL:
    .word (0+(B1_BWSCON<<4)+(B2_BWSCON<<8)+(B3_BWSCON<<12)+(B4_BWSCON<<16)+(B5_BWSCON<<20)+(B6_BWSCON<<24)+(B7_BWSCON<<28))
    .word ((B0_Tacs<<13)+(B0_Tcos<<11)+(B0_Tacc<<8)+(B0_Tcoh<<6)+(B0_Tcah<<4)+(B0_Tacp<<2)+(B0_PMC))
    .word ((B1_Tacs<<13)+(B1_Tcos<<11)+(B1_Tacc<<8)+(B1_Tcoh<<6)+(B1_Tcah<<4)+(B1_Tacp<<2)+(B1_PMC))
    .word ((B2_Tacs<<13)+(B2_Tcos<<11)+(B2_Tacc<<8)+(B2_Tcoh<<6)+(B2_Tcah<<4)+(B2_Tacp<<2)+(B2_PMC))
    .word ((B3_Tacs<<13)+(B3_Tcos<<11)+(B3_Tacc<<8)+(B3_Tcoh<<6)+(B3_Tcah<<4)+(B3_Tacp<<2)+(B3_PMC))
    .word ((B4_Tacs<<13)+(B4_Tcos<<11)+(B4_Tacc<<8)+(B4_Tcoh<<6)+(B4_Tcah<<4)+(B4_Tacp<<2)+(B4_PMC))
    .word ((B5_Tacs<<13)+(B5_Tcos<<11)+(B5_Tacc<<8)+(B5_Tcoh<<6)+(B5_Tcah<<4)+(B5_Tacp<<2)+(B5_PMC))
    .word ((B6_MT<<15)+(B6_Trcd<<2)+(B6_SCAN))
    .word ((B7_MT<<15)+(B7_Trcd<<2)+(B7_SCAN))
    .word ((REFEN<<23)+(TREFMD<<22)+(Trp<<20)+(Tsrc<<18)+REFCNT)
    .word 0xB1
    .word 0x30
    .word 0x30


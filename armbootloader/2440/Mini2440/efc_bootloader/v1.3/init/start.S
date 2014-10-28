
/* 
 *  Embedded Fans Club 
 *  Bootloader 1st stage
 *  Assembly codes to initialize the specific hardware
 */

#define WTCON                           (0x53000000)
#define MPLLCON                         (0x4C000004)
#define CLKDIVN                         (0x4C000014)
#define MINI2440_MPLL_400MHZ            ((0x5C<<12) | (0x01<<4) | (0x01))

#define SP_ADDR                         (0x34000000)

.globl _start
_start:

/*  Disable watchdog */
    ldr r0, =WTCON
    mov r1, #0
    str r1, [r0]
    
/*  Init the clock */
    ldr r0, =CLKDIVN
    mov r1, #0x05               // FCLK:HCLK:PCLK = 1:4:8
    str r1, [r0]

    /* 
     *  If HDIVN is not 0, the cpu bus type change 
     *  from "fast bus mode" to "asynchronous bus mode" 
     */
    mrc p15, 0, r1, c1, c0, 0
    orr r1, r1, #0xc0000000
    mcr p15, 0, r1, c1, c0, 0

    ldr r0, =MPLLCON
    ldr r1, =MINI2440_MPLL_400MHZ
    str r1, [r0]

/*  Init sdram */
    bl lowlevel_init

/*  Relocate */
    ldr sp, =SP_ADDR
    
    bl nand_init_ll

    mov r0, #0
    ldr r1, =_start             /* _start is 0x33F80000(see the efc_boot.lds) */
    ldr r2, =__bss_start
    sub r2, r2, r1
    bl copy2ram

/*  Clear bss */
    bl clear_bss
    
/*  Jump to main */
    ldr pc, =board_init_r

/* System is never returned here!! */

clear_bss:
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0x00000000

1:
    str r2, [r0], #4
    cmp r0, r1
    bne 1b

    mov pc, lr


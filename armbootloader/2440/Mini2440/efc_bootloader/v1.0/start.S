
#define MPLLCON                         (0x4C000004)
#define CLKDIVN                         (0x4C000014)
#define MINI2440_MPLL_400MHZ            ((0x5C<<12) | (0x01<<4) | (0x01))

#define BWSCON	                        (0x48000000)
#define SP_ADDR                         (0x34000000)

.globl _start
_start:

/*  Disable watchdog */
    ldr r0, =0x53000000
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

/*  Init UART0 */
    bl uart0_init

/*  Init sdram */
    adr r0, SMRDATA
    ldr r1, =BWSCON
    add r2, r1, #13*4

0:
    ldr r3, [r0], #4
    str r3, [r1], #4
    cmp r1, r2
    bne 0b

/*  Relocate */
    ldr sp, =SP_ADDR
    
    bl nand_init_ll

    mov r0, #0
    ldr r1, =_start             /* _start is 0x33F80000(see the boot.lds) */
    ldr r2, =__bss_start
    sub r2, r2, r1
    bl copy2ram

/*  Clear bss */
    bl clear_bss
    
/*  Jump to main */
    ldr lr, =infinite_loop
    ldr pc, =armboot_start

infinite_loop:
    b   infinite_loop


clear_bss:
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0x00000000

1:
    str r2, [r0], #4
    cmp r0, r1
    bne 1b

    mov pc, lr

SMRDATA:
    .long 0x22011110     // BWSCON
    .long 0x00000700     // BANKCON0
    .long 0x00000700     // BANKCON1
    .long 0x00000700     // BANKCON2
    .long 0x00000700     // BANKCON3  
    .long 0x00000740     // BANKCON4
    .long 0x00000700     // BANKCON5
    .long 0x00018005     // BANKCON6
    .long 0x00018005     // BANKCON7
    .long 0x008C04F4     // REFRESH
    .long 0x000000B1     // BANKSIZE
    .long 0x00000030     // MRSRB6
    .long 0x00000030     // MRSRB7


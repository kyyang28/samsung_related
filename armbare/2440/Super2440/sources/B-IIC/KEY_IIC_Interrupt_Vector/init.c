
#include "s3c2440.h"
#include "irq.h"


void clock_init(void)
{
    S3C2440_CLOCK * const clock = S3C2440_Get_CLOCK_Base();
	
    //locktime
    clock->LOCKTIME = 0xFFFFFFFF;
	
    //clock divide
    clock->CLKDIVN |= (1<<0 | 1<<2);
    clock->CLKDIVN &= ~(1<<1);

    __asm__ (
        "mrc  p15, 0, r1, c1, c0, 0\n" 
        "orr  r1, r1, #0xC0000000\n" 
        "mcr  p15, 0, r1, c1, c0, 0\n"
    );

    clock->MPLLCON = (MDIV << 12) | (PDIV << 4) | (SDIV << 0);
}


void led_init(void)
{
    // GPF6 LED1  [13:12]   01 = Output
    GPFCON |= 1<<12;
    GPFCON &= ~(1<<13);
    	                        
    // GPG0 LED2    [1:0]       01 = Output
    GPGCON |= 1<<0;
    GPGCON &= ~(1<<1);
    	                                        
    // GPG1 LED3    [3:2]       01 = Output
    GPGCON |= 1<<2;
    GPGCON &= ~(1<<3);
    	                                                        
    // GPG10 LED4   [21:20]     01 = Output
    GPGCON |= 1<<20;
    GPGCON &= ~(1<<21);
    	
    // light all off
    GPFDAT |= 1<<6;
    GPGDAT |= (1<<0 | 1<<1 | 1<<10);
}


void nand_init(void)
{
    S3C2440_NAND_REG * const s3c2440nand = (S3C2440_NAND_REG *)0x4E000000;
	
    s3c2440nand->NFCONF = (TACLS<<12) | (TWRPH0<<8) | (TWRPH1<<4);
    s3c2440nand->NFCONT = (1<<4) | (1<<1) | (1<<0);

    extern void nand_reset(void);
    nand_reset();
}


static void key_interrupt_init(void)
{
    /*
     *	GPFCON  0x56000050  R/W  Configures the pins of port F 
     *	
     *	GPF1  [3:2]  		00 = Input           01 = Output 
     *				10 = EINT[1]         11 = Reserved
     *
     *	Key 2 
     */
    GPFCON &= ~(1<<2); 
    GPFCON |= 1<<3;

    /*
     *	GPFCON  0x56000050  R/W  Configures the pins of port F 
     *	
     *	GPF3  [7:6]  	00 = Input               01 = Output 
     *			10 = EINT[3]             11 = Reserved
     *
     *	Key 3 
     */
    GPFCON |= 1<<7; 
    GPFCON &= ~(1<<6);

	
    /*
     *	GPFCON  0x56000050  R/W  Configures the pins of port F 
     *
     *	GPF5  [11:10]  	00 = Input               01 = Output 
     *			10 = EINT[5]             11 = Reserved 
     *
     *	Key 4 
     */
    GPFCON |= 1<<11;
    GPFCON &= ~(1<<10);


    /*
     *	GPFCON  0x56000050  R/W  Configures the pins of port F 
     *
     *	GPF0  [1:0]  	00 = Input               01 = Output 
     *			10 = EINT[0]             11 = Reserved 
     *
     *	Key 5 
     */
    GPFCON |= 1<<1;
    GPFCON &= ~(1<<0);

    // GPF2 -- EINT2
    GPFCON |= 1<<5;
    GPFCON &= ~(1<<4);
	
    // GPF4 -- EINT4
    GPFCON |= 1<<9;
    GPFCON &= ~(1<<8);
}


void irq_init(void)
{
    
    int i;
    for(i = 0; i < 32; ++i)
        isr_vector[i] = dummy_ISR;

    key_interrupt_init();

    //INTMSK = 0xFFFFFFFF; // disable all interrupts
    
    /* 
     * External Interrupt Configuration *
     * Only need to enable EINT4 and EINT5, since [3:0] Reserved
     */
    EINTMASK &= ~(1<<4 | 1<<5);
		
    /* Interrupt Mode -- IRQ or FIQ */
    INTMOD &= ~(1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4);	// Choose IRQ
		
    /* Interrupt Mask -- Interrupt service is available or not */
    INTMSK &= ~(1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4);
    
    /* IIC irq mode */
    INTMOD &= ~(1<<27);
    

    /* IIC mask */
    INTMSK &= ~(1<<27);
 
    /* Enable irq bit in the CPSR register */
    __asm__ (
        "msr cpsr_c, #0x5F\n"
    );

    extern void do_k2(void);
    extern void do_k3(void);
    extern void do_k5(void);
    extern void do_k6(void);
    extern void do_k4_k7(void);
    extern void do_IIC_ISR(void);
    
    isr_vector[EINT0_OFFSET] = do_k5;
    isr_vector[EINT1_OFFSET] = do_k2;
    isr_vector[EINT2_OFFSET] = do_k6;
    isr_vector[EINT3_OFFSET] = do_k3;
    isr_vector[EINT4_OFFSET] = do_k4_k7;
    isr_vector[IIC_OFFSET] = do_IIC_ISR;
}


int CopyCode2RAM(unsigned char *buf, unsigned long start_addr, int size)
{
    extern int nand_read(unsigned char *buf, unsigned long start_addr, int size);
    nand_read(buf, start_addr, size);
    return 0;
}


void clear_bss(void)
{
    extern int __bss_start, __bss_end;
    int *p = &__bss_start;

    while(p < &__bss_end) {
        *p = 0;
        p++;
    }
}



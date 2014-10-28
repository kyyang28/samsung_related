
#include "s3c2440.h"
#include "interrupt.h"


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


void nand_init(void)
{
    S3C2440_NAND_REG * const s3c2440nand = (S3C2440_NAND_REG *)0x4E000000;
	
    s3c2440nand->NFCONF = (TACLS<<12) | (TWRPH0<<8) | (TWRPH1<<4);
    s3c2440nand->NFCONT = (1<<4) | (1<<1) | (1<<0);

    extern void nand_reset(void);
    nand_reset();
}


void irq_init(void)
{
    int i;
    for(i = 0; i < NUMOFISR; ++i)
        ISR_VECTOR[i] = dummy_ISR;

    /* IIC irq mode */
    INTMOD &= ~(1<<27);

    /* IIC mask */
    INTMSK = 0xFFFFFFFF; // disable all interrupts
    INTMSK &= ~(1<<27);

    /* Enable irq bit in the CPSR register */
    __asm__ (
        "msr cpsr_c, #0x5F\n"
    );
    
    extern void IIC_int_handler(void);
    ISR_VECTOR[IIC_INTOFFSET] = IIC_int_handler; 
}


int CopyCode2RAM(unsigned char *buf, unsigned long start_addr, int size)
{
    extern int nand_read(unsigned char *buf, unsigned long start_addr, int size);
    nand_read(buf, start_addr, size);
    return 0;
}

#if 0
void clear_bss(void)
{
    extern int __bss_start, __bss_end;
    int *p = &__bss_start;

    while(p < &__bss_end) {
        *p = 0;
        p++;
    }
}
#endif


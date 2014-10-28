
#include "s3c2440.h"
#include "irq.h"


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
    GPFDAT = 1<<6;
    GPGDAT = (1<<0 | 1<<1 | 1<<10);
}


void timer0_init(void)
{
    TCFG0 = 99;    /* Prescaler */
    TCFG1 = 0x3;   /* MUX : 0011 = 1/16 */
    TCNTB0 = 15625; // 2.0833s
    // The initial value of TCMPB0 is 0, so we don't need to setup manually.
    
    /* 1<<0 (Timer0 start/stop), Timer0 manual update, Timer0 auto reload on/off */
    TCON |= (1<<1);
    TCON = 0x09; // TCON = 0x0D is fine(Timer0 output invertor on/off)
}


void dummy_isr(void)
{
    while(1);
}


void irq_init(void)
{
    int i;
    for(i = 0; i < 60; ++i)
        irq_vector[i] = dummy_isr;

    INTMOD &= ~(1<<10);
    INTMSK &= ~(1<<10);

    __asm__ (
        "msr cpsr_c, #0x5F\n"
    );

    extern void do_timer0(void);
    irq_vector[TIMER0_OFT] = do_timer0;
}


void copy_from_SRAM_to_SDRAM(void)
{
    unsigned int *SRAM_ADDR = (unsigned int *)0;
    unsigned int *SDRAM_ADDR = (unsigned int *)0x30000000;

    for( ; SRAM_ADDR < (unsigned int *)4096; SRAM_ADDR++, SDRAM_ADDR++)
        *SDRAM_ADDR = *SRAM_ADDR;
}



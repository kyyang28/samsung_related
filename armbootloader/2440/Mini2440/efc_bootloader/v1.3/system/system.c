
/*
 *  EFC System.c to do the system kind of stuff
 */


#include "../include/mini2440_regs.h"

/*
 * CR1 bits (CP#15 CR1)
 */
#define CR_I	(1 << 12)	/* Icache enable			*/
#define isb() __asm__ __volatile__ ("" : : : "memory")
#define nop() __asm__ __volatile__("mov\tr0,r0\t@ nop\n\t");

static void cp_delay (void)
{
	volatile int i;

	/* copro seems to need some delay between reading and writing */
	for (i = 0; i < 100; i++)
		nop();
	asm volatile("" : : : "memory");
}

static inline unsigned int get_cr(void)
{
	unsigned int val;
	asm("mrc p15, 0, %0, c1, c0, 0	@ get CR" : "=r" (val) : : "cc");
	return val;
}

static inline void set_cr(unsigned int val)
{
	asm volatile("mcr p15, 0, %0, c1, c0, 0	@ set CR"
	  : : "r" (val) : "cc");
	isb();
}

static void cache_enable(unsigned int cache_bit)
{
	unsigned int reg;

	reg = get_cr();	/* get control reg. */
	cp_delay();
	set_cr(reg | cache_bit);
}

int icache_enable(void)
{
	cache_enable(CR_I);
    return 0;
}

int disable_interrupts (void)
{
	unsigned long old,temp;
	__asm__ __volatile__("mrs %0, cpsr\n"
			     "orr %1, %0, #0xc0\n"
			     "msr cpsr_c, %1"
			     : "=r" (old), "=r" (temp)
			     :
			     : "memory");
	return (old & 0x80) == 0;
}

void reset_cpu(void)
{
	/* Disable watchdog */
	WTCON = 0x0;

	/* Initialize watchdog timer count register */
    WTCNT = 0x0001;

	/* Enable watchdog timer; assert reset at timer timeout */
    WTCON = 0x0021;

	while (1)
		/* loop forever and wait for reset to happen */;

	/*NOTREACHED*/
}

/* Make compilation w/o error */
int raise (int signum)
{
    return 0;
}





/* Interrupt Sources */
#define GPGCON								(*(volatile unsigned long *)0x56000060)
#define GPGDAT								(*(volatile unsigned long *)0x56000064)

#define EXTINT1								(*(volatile unsigned long *)0x5600008C)
#define EXTINT2								(*(volatile unsigned long *)0x56000090)
#define EINTMASK							(*(volatile unsigned long *)0x560000A4)
#define EINTPEND							(*(volatile unsigned long *)0x560000A8)

/* Interrupt Controller */
#define SRCPND								(*(volatile unsigned long *)0x4A000000)
#define INTMOD								(*(volatile unsigned long *)0x4A000004)
#define INTMSK								(*(volatile unsigned long *)0x4A000008)
#define PRIORITY							(*(volatile unsigned long *)0x4A00000C)
#define INTPND								(*(volatile unsigned long *)0x4A000010)
#define INTOFFSET							(*(volatile unsigned long *)0x4A000014)
#define SUBSRCPND							(*(volatile unsigned long *)0x4A000018)
#define INTSUBMSK							(*(volatile unsigned long *)0x4A00001C)


extern void leds_on(int num);
extern void leds_off(int num);

void key_int_init(void)
{
	/* 1. Setup the interrupt source(keys) */
	GPGCON |= (0x2<<22) | (0x2<<14) | (0x2<<12) | (0x2<<10) | (0x2<<6) | (0x2);
	
	/* Trigger method: Both edge triggered */
	EXTINT1 = (0x7<<28) | (0x7<<24) | (0x7<<20) | (0x7<<12) | (1<<3) | (0x7);
	EXTINT2 = 0x7<<12;
	
	/* External Interrupt Mask Register */
	EINTMASK &= ~(1<<19 | 1<<15 | 1<<14 | 1<<13 | 1<<11 | 1<<8);
	
	/* 2. Setup the interrupt controller */
	INTMOD &= ~(1<<5);
	INTMSK &= ~(1<<5);
	
	/* 3. Enable the irq bit in CPSR */
	__asm__ (
		"msr cpsr_c, #0x53\n"
	);
}

void key_int_handler(void)
{
	unsigned long intoft = INTOFFSET;
	
	if (EINTPEND & (1<<8)) {
		if (GPGDAT & (1<<0))
			leds_off(1);
		else 
			leds_on(1);
	}

	if (EINTPEND & (1<<11)) {
		if (GPGDAT & (1<<3))
			leds_off(2);
		else
			leds_on(2);
	}
	
	if (EINTPEND & (1<<13)) {
		if (GPGDAT & (1<<5))
			leds_off(3);
		else
			leds_on(3);
	}

	if (EINTPEND & (1<<14)) {
		if (GPGDAT & (1<<6))
			leds_off(4);
		else
			leds_on(4);
	}

	if (EINTPEND & (1<<15)) {
		if (GPGDAT & (1<<7)) {
			leds_off(1);
			leds_off(3);
		}else {
			leds_on(1);
			leds_on(3);
		}
	}

	if (EINTPEND & (1<<19)) {
		if (GPGDAT & (1<<11)) {
			leds_off(2);
			leds_off(4);
		}else {
			leds_on(2);
			leds_on(4);
		}
	}

	EINTPEND = (1<<8) | (1<<11) | (1<<13) | (1<<14) | (1<<15) | (1<<19);

	SRCPND = 1<<intoft;
	INTPND = 1<<intoft;
}


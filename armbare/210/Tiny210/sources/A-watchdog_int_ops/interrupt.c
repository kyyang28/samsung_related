
#include "stdio.h"
#include "interrupt.h"
#include "wdt_ops.h"

static int IC_init_ll(void);
static void do_undefined_instruction(void);
static void do_software_interrupt(void);
static void do_prefetch_abort(void);
static void do_data_abort(void);
static void do_reserved(void);
extern void do_irq(void);
static void do_fiq(void);
static void exceptionVectorInit(void);
static void vicIntSelectIRQ(unsigned long irqNum);
static void vicIntSelectFIQ(unsigned long irqNum);
static void vicIntEnable(unsigned long irqNum);
static void vicIntDisable(unsigned long irqNum);
static void vicIntEnClear(void);
void clearVecAddress(void);
static void vicVectAddr(unsigned long irqNum, void (*isr)(void));

int interrupt_init_ll(void)
{
	/* SEQ 0: Exception vector init */
	exceptionVectorInit();
	
	/* SEQ 1: Interrupt Sources init */
	wdt_init_ll();

	/* SEQ 2: Interrupt Controller init */
	IC_init_ll();
	
	return 0;
}

/* Interrupt Controller(IC) Init */
static int IC_init_ll(void)
{
	/* Disable all interrupts */
	vicIntEnClear();

	/* Set the interrupt type to IRQ */	
	vicIntSelectIRQ(VIC0_WDT);

	/* Enable the interrupt request line of the VIC0 */
	vicIntEnable(VIC0_WDT);

	/* Setup the ISR vector address */
	vicVectAddr(VIC0_WDT, wdt_irq_handler);

	return 0;
}

static void exceptionVectorInit(void)
{
	EXCEPTION_UNDEF_ENTRY			= (unsigned long)do_undefined_instruction;
	EXCEPTION_SWI_ENTRY				= (unsigned long)do_software_interrupt;
	EXCEPTION_PREFETCHABORT_ENTRY	= (unsigned long)do_prefetch_abort;
	EXCEPTION_DATAABORT_ENTRY		= (unsigned long)do_data_abort;
	EXCEPTION_RESERVED_ENTRY		= (unsigned long)do_reserved;
	EXCEPTION_IRQ_ENTRY				= (unsigned long)do_irq;
	EXCEPTION_FIQ_ENTRY				= (unsigned long)do_fiq;
	return;
}

static void do_undefined_instruction(void)
{
	printf("undefined instruction!\r\n");
	while (1);
	
	/* Never reach here! */
	return;
}

static void do_software_interrupt(void)
{
	printf("software interrupt!\r\n");
	while (1);
	
	/* Never reach here! */
	return;
}

static void do_prefetch_abort(void)
{
	printf("prefetch abort!\r\n");
	while (1);
	
	/* Never reach here! */
	return;
}

static void do_data_abort(void)
{
	printf("data abort!\r\n");
	while (1);
	
	/* Never reach here! */
	return;
}

static void do_reserved(void)
{
	printf("reserved!\r\n");
	while (1);
	
	/* Never reach here! */
	return;
}

static void do_fiq(void)
{
	printf("fiq!\r\n");
	while (1);

	/* Never reach here! */
	return;
}

static void vicIntSelectIRQ(unsigned long irqNum)
{
	/* For VIC0 */
	if (irqNum < 32)
		VIC0INTSELECT &= ~(1<<irqNum);

	/* For VIC1 */
	else if (irqNum < 64)
		VIC1INTSELECT &= ~(1<<irqNum);

	/* For VIC2 */
	else if (irqNum < 96)
		VIC2INTSELECT &= ~(1<<irqNum);

	/* For VIC3 */
	else
		VIC3INTSELECT &= ~(1<<irqNum);

	return;
}

static void vicIntSelectFIQ(unsigned long irqNum)
{
	/* For VIC0 */
	if (irqNum < 32)
		VIC0INTSELECT |= (1<<irqNum);

	/* For VIC1 */
	else if (irqNum < 64)
		VIC1INTSELECT |= (1<<irqNum);

	/* For VIC2 */
	else if (irqNum < 96)
		VIC2INTSELECT |= (1<<irqNum);

	/* For VIC3 */
	else
		VIC3INTSELECT |= (1<<irqNum);

	return;
}

static void vicIntEnable(unsigned long irqNum)
{
	/* For VIC0 */
	if (irqNum < 32)
		VIC0INTENABLE |= (1<<irqNum);

	/* For VIC1 */
	else if (irqNum < 64)
		VIC1INTENABLE |= (1<<irqNum);
	
	/* For VIC2 */
	else if (irqNum < 96)
		VIC2INTENABLE |= (1<<irqNum);
	
	/* For VIC3 */
	else
		VIC3INTENABLE |= (1<<irqNum);

	return;
}

static void vicIntDisable(unsigned long irqNum)
{
	/* For VIC0 */
	if (irqNum < 32)
		VIC0INTENABLE &= ~(1<<irqNum);

	/* For VIC1 */
	else if (irqNum < 64)
		VIC1INTENABLE &= ~(1<<irqNum);

	/* For VIC2 */
	else if (irqNum < 96)
		VIC2INTENABLE &= ~(1<<irqNum);

	/* For VIC3 */
	else
		VIC3INTENABLE &= ~(1<<irqNum);

	return;
}

static void vicIntEnClear(void)
{
	VIC0INTENCLEAR = 0xFFFFFFFF;
	VIC1INTENCLEAR = 0xFFFFFFFF;
	VIC2INTENCLEAR = 0xFFFFFFFF;
	VIC3INTENCLEAR = 0xFFFFFFFF;
	return;
}

void clearVecAddress(void)
{
	VIC0ADDRESS = 0;
	VIC1ADDRESS = 0;
	VIC2ADDRESS = 0;
	VIC3ADDRESS = 0;
}

static void vicVectAddr(unsigned long irqNum, void (*isr)(void))
{
	/* For VIC0 */
	if (irqNum < 32)
		(*(volatile unsigned long *)(VIC0VECTADDR_BASE + irqNum*4))			= (unsigned long)isr;
	
	/* For VIC1 */
	else if (irqNum < 64)
		(*(volatile unsigned long *)(VIC1VECTADDR_BASE + (irqNum-32)*4))	= (unsigned long)isr;
	
	/* For VIC2 */
	else if (irqNum < 96)
		(*(volatile unsigned long *)(VIC2VECTADDR_BASE + (irqNum-64)*4))	= (unsigned long)isr;

	/* For VIC3 */
	else
		(*(volatile unsigned long *)(VIC3VECTADDR_BASE + (irqNum-96)*4))	= (unsigned long)isr;
	
	return;
}

static unsigned long getVICIRQStatus(unsigned long vicChannel)
{
	if (0 == vicChannel)
		return VIC0IRQSTATUS;
	else if (1 == vicChannel)
		return VIC1IRQSTATUS;
	else if (2 == vicChannel)
		return VIC2IRQSTATUS;
	else if (3 == vicChannel)
		return VIC3IRQSTATUS;
	else
		printf("wrong channel, please choose 0,1,2,3\r\n");
	return 0;
}

void irq_handler(void)
{
	int i = 0;
	unsigned long vicAddress[4] = {VIC0ADDRESS, VIC1ADDRESS, VIC2ADDRESS, VIC3ADDRESS};
	void (*isr)(void) = NULL;

	while (i < 4) {
		if (getVICIRQStatus(i) != 0) {
			isr = (void (*)(void))vicAddress[i];
			break;
		}
		i++;
	}

	/* Execute the isr function */
	(*isr)();
}


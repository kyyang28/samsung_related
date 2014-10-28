
#include "s3c2440-regs.h"
#include "int_irq_vectors.h"
#include "key_irq.h"
#include "timer0_irq.h"
#include "rtc_irq.h"
#include "ts_irq.h"
#include "i2c_irq.h"

static void int_controller_init_ll(void)
{
	/* Setup the interrupt controller */
	INTMOD &= ~(EINT8_23 | INT_TIMER0 | INT_RTC | INT_ADC);
	INTMSK &= ~(EINT8_23 | INT_TIMER0 | INT_RTC | INT_ADC);
	INTSUBMSK &= ~(INT_ADC_S | INT_TC);
}

static void int_vectors_init_ll(void)
{
	int i;
	for (i = 0; i < 60; ++i)
		isr[i] = dummyISR;

	isr[EINT8_23_OFT]	= key_isr;
	isr[INT_TIMER0_OFT]	= timer0_isr;
	isr[INT_RTC_OFT]	= rtc_isr;
	isr[INT_ADC_OFT]	= ts_isr;
	isr[INT_IIC_OFT]	= i2c_isr;
}

int int_irq_init_ll(void)
{
	/* Step 1: Interrupt Sources initializations */
	key_int_init_ll();
	timer0_int_init_ll();
	rtc_int_init_ll();
	ts_int_init_ll();
	i2c_int_init_ll();

	/* Step 2: Interrupt Controller initialization */
	int_controller_init_ll();

	/* Interrupt Vectors Initialization */
	int_vectors_init_ll();
	
	/* Step 3: Enable the I-bit of CPSR */
	__asm__ (
		"mrs r0, cpsr\n"
		"and r0, r0, #~(1<<7)\n"
		"msr cpsr, r0\n"
	);

	return 0;
}

void int_vec_handler(void)
{
	u32 intoft = INTOFFSET;

	isr[intoft]();
	
	SRCPND = 1<<intoft;
	INTPND = 1<<intoft;
}


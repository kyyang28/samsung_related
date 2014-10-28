
#include "s3c2440-regs.h"
#include "uart_ops.h"
#include "printf.h"

static void tc_isr(void)
{
	if (!(ADCDAT0 & (1<<15))) {
		/* Pen down */
		serial_printf("\r\nPen down!\r\n");
		
		/* Auto mode */
		ADCTSC = (1<<3) | (1<<2) | (0x00<<0);

		/* Start ADC */
		ADCCON |= (1<<0);

	}else {
		/* Pen up */
		serial_printf("\r\nPen up!\r\n");
		
		/* Wait Pen down interrupt mode */
		ADCTSC = (0<<8) | (1<<7) | (1<<6) | (0<<5) | (1<<4) | (0<<3) | (0x3);	/* ADCTSC = 0xd3 */
	}

	/* Clear INT_ADC interrupt */
	SUBSRCPND = INT_TC;
	SRCPND = INT_ADC;
	INTPND = INT_ADC;
}

static void adc_isr(void)
{
	/* Print out the x,y axis values */
	serial_printf("(x,y) = (%d,%d)\r\n", (int)(ADCDAT0 & 0x3ff), (int)(ADCDAT1 & 0x3ff));

	/* Wait Pen up interrupt mode */
	ADCTSC = (1<<8) | (1<<7) | (1<<6) | (0<<5) | (1<<4) | (0<<3) | (0x3);	/* ADCTSC = 0xd3 */
	
	/* Clear INT_ADC interrupt */
	SUBSRCPND = INT_ADC_S;
	SRCPND = INT_ADC;
	INTPND = INT_ADC;
}

void ts_isr(void)
{
	unsigned long intoft = INTOFFSET;

	if (intoft == 31) {
		if (SUBSRCPND & INT_TC)
			tc_isr();

		if (SUBSRCPND & INT_ADC_S)
			adc_isr();
	}
	
	SRCPND = 1<<intoft;
	INTPND = 1<<intoft;
}

static void ts_init_ll(void)
{
	ADCCON = (1<<14) | (49<<6);
	ADCDLY = 50000;

	/* Wait Pen down interrupt mode */
	ADCTSC = (0<<8) | (1<<7) | (1<<6) | (0<<5) | (1<<4) | (0<<3) | (0x3);	/* ADCTSC = 0xd3 */
}

static void adc_ts_irq_enable(void)
{
	INTMSK &= ~INT_ADC;
	INTSUBMSK &= ~INT_ADC_S;
	INTSUBMSK &= ~INT_TC;
}

static void adc_ts_irq_disable(void)
{
	INTMSK |= INT_ADC;
	INTSUBMSK |= INT_ADC_S;
	INTSUBMSK |= INT_TC;
}

void ts_test(void)
{
	/* Enable adc_ts interrupt controller */
	adc_ts_irq_enable();

	/* Touch screen initialization */
	ts_init_ll();

	serial_printf("\r\nTouch screen test program, press any key to exit\r\n");
	serial_getc_ll();	
	
	/* Disable adc_ts interrupt controller */
	adc_ts_irq_disable();
}


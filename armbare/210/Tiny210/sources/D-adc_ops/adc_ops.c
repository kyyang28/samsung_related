
#include "stdio.h"
#include "adc_ops.h"

static int adc_init_ll(void)
{
	TSADCCON0 = (1<<16) | (1<<14) | (65<<6);

	TSADCCON0 &= ~(1<<2 | 1<<1);

	ADCMUX = 0;

	TSADCCON0 |= (1<<0);

	while (TSADCCON0 & (1<<0));

	/* Check the end of ADC conversion */
	while (!(TSADCCON0 & (1<<15)));

	return 0;
}

static int adc_read_ll(void)
{
	return (TSDATX0 & 0xfff);
}

void adc_test_ll(void)
{
	printf("Adc testing program!\r\n");	
	adc_init_ll();
	adc_read_ll();
}


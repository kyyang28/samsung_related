

#include "s3c2440-regs.h"


extern int serial_printf(const char * format, ...);
extern void leds_on(int num);
extern void leds_off(int num);
extern void buzzer_on(void);
extern void buzzer_off(void); 
extern unsigned char awaitkey(unsigned long timeout);

int read_adc(int channel, u32 preScaler)
{
	ADCCON = (1<<14) | (preScaler<<6) | (channel<<3);
	ADCCON |= 0x1;

	while (!(ADCCON & (1<<15)));
	
	return ((int)ADCDAT0 & 0x3ff);
}


void adc_ops(void)
{
	int adc_val = 0;
	int sel_mux = 0;
	volatile u32 preScaler;
	
	preScaler = PCLK/ADC_FREQ - 1;
	serial_printf("\r\npreScaler value = %d\r\n\r\n", preScaler);

	serial_printf("\r\nadc test is starting...\r\n");
	while (!awaitkey(0)) {
		adc_val = read_adc(sel_mux, preScaler);
		serial_printf( "AIN%d: %d\r\n", sel_mux, adc_val);

		if (adc_val >= 200 && adc_val < 400)
			leds_on(1);
		else
			leds_off(1);
			
		if (adc_val >= 400 && adc_val < 600)
			leds_on(2);
		else
			leds_off(2);
		
		if (adc_val >= 600 && adc_val < 800)
			leds_on(3);
		else 
			leds_off(3);
		
		if (adc_val >= 800 && adc_val < 1000)
			leds_on(4);
		else 
			leds_off(4);
	
		if (adc_val <= 100 || adc_val >= 1000) 
			buzzer_on();
		else 
			buzzer_off();

		DELAY(0x100000);
	}
	
	serial_printf("\r\nadc test is finished!\r\n");
}


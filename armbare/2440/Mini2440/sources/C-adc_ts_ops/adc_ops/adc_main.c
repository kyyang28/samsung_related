

extern void uart_init_ll(void);
extern void leds_buzzer_init_ll(void);
extern void adc_ops(void);

int adc_main(void)
{
	/* Uart initialization */
	uart_init_ll();
	
	/* Leds and buzzer initializations */
	leds_buzzer_init_ll();
	
	/* Test the AD Converter */
	adc_ops();
	
	return 0;
}


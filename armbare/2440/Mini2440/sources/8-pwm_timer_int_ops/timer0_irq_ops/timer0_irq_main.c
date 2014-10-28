

extern void leds_init(void);
extern void leds_all_off(void);

int timer0_irq_main(void)
{
	/* leds initialization */
	leds_init();
	leds_all_off();

	while (1);
	return 0;
}


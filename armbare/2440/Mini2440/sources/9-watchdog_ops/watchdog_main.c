
extern void leds_init(void);
extern void leds_on(int num);
extern void leds_off(int num);

void delay(void)
{
	volatile int i;
	for (i = 0; i < 50000; i++);
}

int watchdog_main(void)
{
	leds_init();

	leds_on(1);
	delay();
	leds_off(1);

	return 0;
}


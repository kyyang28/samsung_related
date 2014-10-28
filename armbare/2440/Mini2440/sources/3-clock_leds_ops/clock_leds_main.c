
/* leds gpio registers */
#define GPBCON						(*(volatile unsigned long *)0x56000010)
#define GPBDAT						(*(volatile unsigned long *)0x56000014)
#define GPBUP						(*(volatile unsigned long *)0x56000018)

void delay(void)
{
	volatile int i;
	for (i = 0; i < 50000; i++);
}

void leds_init(void)
{
	GPBCON = (0x1<<10) | (0x1<<12) | (0x1<<14) | (0x1<<16);
	/* leds all off */
	GPBDAT = 0xF<<5;
}

void leds_ops(void)
{
	unsigned int i = 0;

	while (1) {
		GPBDAT = ~(i<<5);
		if (++i == 16)
			i = 0;
		delay();
	}
}

int clock_led_main(void)
{
	/* leds initialization */
	leds_init();

	/* leds operations */
	leds_ops();

	return 0;
}


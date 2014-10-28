
#define GPBCON							(*(volatile unsigned long *)0x56000010)
#define GPBDAT							(*(volatile unsigned long *)0x56000014)
#define GPBUP							(*(volatile unsigned long *)0x56000018)

/* 
 *	IMPORTANT: Don't forget to include volatile,
 *	otherwise, the program is not running properly
 */
void delay(void)
{
	volatile int i;
	for (i = 0; i < 50000; i++);
}

void leds_init(void)
{
	/* Setup the GPB5,6,7,8 pins to be OUTPUT  */
	GPBCON = 0x1<<10 | 0x1<<12 | 0x1<<14 | 0x1<<16;

	/* Setup the level of GPB5,6,7,8 to be high(1) */
	GPBDAT = 0xF<<5;
}

void leds_run(void)
{
	unsigned int i = 0;

	while (1) {
		GPBDAT = ~(i<<5);
		if (++i == 16)
			i = 0;
		delay();
	}
}

/* num is 1,2,3,4 */
void leds_on(int num)
{
	GPBDAT &= ~(1<<(num+4));
}

/* num is 1,2,3,4 */
void leds_off(int num)
{
	GPBDAT |= (1<<(num+4));
}

void leds_ops(void)
{
	leds_run();
}

int leds_main(void)
{
	/* leds initialization */
	leds_init();
	
	/* leds operations */
	leds_ops();

	return 0;
}


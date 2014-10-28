
#define GPJ2CON							(*(volatile unsigned long *)0xE0200280)
#define GPJ2DAT							(*(volatile unsigned long *)0xE0200284)

/* 
 *	IMPORTANT NOTICE: 
 *	If the gcc including the optimization
 *	option such as -O1, -Os, etc, the "volatile" 
 *	keyword must be added by the delay function, 
 *	otherwise the program won't be running properly
 */
static void delay(void)
{
	volatile int i;
	for (i = 0; i < 600000; i++);
}

int leds_init_ll(void)
{
	/* Setup the LED's Pins to be output */
	GPJ2CON |= 0x1<<12 | 0x1<<8 | 0x1<<4 | 0x1<<0;
	GPJ2DAT |= 0xF;
	return 0;
}

/*
 *	IMPORTANT
 *	Makesure to use "&=" operator to light on leds,
 *	otherwise all leds will light on.
 */
void leds_on(int num)
{
	GPJ2DAT &= ~(1<<(num-1));
}

/*
 *	IMPORTANT
 *	Makesure to use "|=" operator to light off leds.
 */
void leds_off(int num)
{
	GPJ2DAT |= (1<<(num-1));
}

void leds_all_on(void)
{
	GPJ2DAT &= ~(0xF);
}

void leds_all_off(void)
{
	GPJ2DAT |= 0xF;
}

void leds_run(void)
{
	volatile int i = 0;

	while (1) {
		GPJ2DAT = ~(i<<0);
		if (16 == ++i)
			i = 0;
		delay();
	}
}


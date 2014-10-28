
#define GPKCON0						(*(volatile unsigned long *)0x7F008800)
#define GPKDAT						(*(volatile unsigned long *)0x7F008808)


void delay(void)
{
	volatile unsigned int i;
	for (i = 0; i < 8000000; i++);
}

void led_init(void)
{
	/* 
	 *	GPK4 [19:16]		0001 = Output 
	 *	GPK5 [23:20]		0001 = Output 
	 *	GPK6 [27:24]		0001 = Output 
	 *	GPK7 [31:28]		0001 = Output
	 *	Setup to output pin
	 */
	GPKCON0 = 0x11110000;
}

void led_run(void)
{
	while (1) {
		GPKDAT = ~(1<<4 | 1<<7);
		delay();
		GPKDAT = ~(1<<5 | 1<<6);
		delay();
	}
}

int myled(void)
{
	led_init();
	led_run();

	return 0;
}



#define GPMCON						(*(volatile unsigned long *)0x7F008820)
#define GPMDAT						(*(volatile unsigned long *)0x7F008824)
#define GPMPUD						(*(volatile unsigned long *)0x7F008828)

#ifdef BOOT_FROM_NAND
void delay(void)
{
	volatile unsigned int i;
	for (i = 0; i < 50000; i++);
}
#else	
/* BOOT_FROM_SDCARD delay */
void delay(int loop)
{
	volatile unsigned int i, j;
	for (i = 0; i < 0x100000; i++)
		for (j = 0; j < loop; j++)
			;
}
#endif

void led_init(void)
{
	/* 
	 *	GPM0 [3:0]		0001 = Output 
	 *	GPM1 [7:4]		0001 = Output 
	 *	GPM2 [11:8]		0001 = Output 
	 *	GPM3 [15:12]	0001 = Output
	 *	Setup to output pin
	 */
	GPMCON = 0x1111;

	/* light off all leds */
	GPMDAT = 0xF;
}

void led_run(void)
{
	volatile unsigned int i;
	while (1) {
		for (i = 0; i < 4; i++) {
			GPMDAT = ~(1<<i);
#ifdef BOOT_FROM_NAND
			delay();
#else
			delay(10);
#endif
		}
	}
}

int myled(void)
{
	led_init();
	led_run();

	return 0;
}


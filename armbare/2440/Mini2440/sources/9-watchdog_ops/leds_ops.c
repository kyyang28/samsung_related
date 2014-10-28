
#define GPBCON						(*(volatile unsigned long *)0x56000010)
#define GPBDAT						(*(volatile unsigned long *)0x56000014)

void leds_init(void)
{
	GPBCON = (0x01<<16) | (0x01<<14) | (0x01<<12) | (0x01<<10);
	GPBDAT = 0xf<<5;
}

void leds_on(int num)
{
	if (1 == num || 2 == num || 3 == num || 4 == num)
		GPBDAT &= ~(1<<(num+4));
}

void leds_off(int num)
{
	if (1 == num || 2 == num || 3 == num || 4 == num)
		GPBDAT |= (1<<(num+4));
}


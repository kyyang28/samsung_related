
/* LED */
#define GPKCON0						(*(volatile unsigned long *)0x7F008800)
#define GPKDAT						(*(volatile unsigned long *)0x7F008808)

/* KEY */
#define GPNCON						(*(volatile unsigned long *)0x7F008830)
#define GPNDAT						(*(volatile unsigned long *)0x7F008834)
#define GPNPUD						(*(volatile unsigned long *)0x7F008838)

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

/* 
 *	led_on(1): 1st led on 
 *	led_on(2): 2nd led on 
 *	led_on(3): 3rd led on 
 *	led_on(4): 4th led on 
 */
void led_on(int num)
{
	GPKDAT &= ~(1<<(num+3));
}

/* 
 *	led_off(1): 1st led off 
 *	led_off(2): 2nd led off 
 *	led_off(3): 3rd led off 
 *	led_off(4): 4th led off 
 */
void led_off(int num)
{
	GPKDAT |= (1<<(num+3));
}

void key_init(void)
{
	/*
	 *	GPN0   [1:0]	00 = Input
	 *	GPN1   [3:2]	00 = Input
	 *	GPN2   [5:4]	00 = Input
	 *	GPN3   [7:6]	00 = Input
	 *	GPN4   [9:8]	00 = Input
	 *	GPN5   [11:10]	00 = Input
	 */
	GPNCON = 0x0;
}

void key_test(void)
{
	while (1) {
		/* S1 */
		if (GPNDAT & (1<<0))
			led_off(1);
		else
			led_on(1);
		
		/* S2 */
		if (GPNDAT & (1<<1))
			led_off(2);
		else
			led_on(2);

		/* S3 */
		if (GPNDAT & (1<<2))
			led_off(3);
		else
			led_on(3);
	
		/* S4 */
		if (GPNDAT & (1<<3))
			led_off(4);
		else
			led_on(4);

		/* S5 */
		if (GPNDAT & (1<<4)) {
			led_off(1);
			led_off(3);
		}else {
			led_on(1);
			led_on(3);
		}

		/* S6 */
		if (GPNDAT & (1<<5)) {
			led_off(2);
			led_off(4);
		}else {
			led_on(2);
			led_on(4);
		}
	}
}

int mykey(void)
{
	led_init();
	key_init();
	key_test();
	
	return 0;
}



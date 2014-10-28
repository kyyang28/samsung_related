

/* leds gpio registers */
#define GPBCON						(*(volatile unsigned long *)0x56000010)
#define GPBDAT						(*(volatile unsigned long *)0x56000014)
#define GPBUP						(*(volatile unsigned long *)0x56000018)

/* keys gpio registers */
#define GPGCON						(*(volatile unsigned long *)0x56000060)
#define GPGDAT						(*(volatile unsigned long *)0x56000064)
#define GPGUP						(*(volatile unsigned long *)0x56000068)

/* ###################### Start of leds ######################### */
void leds_init(void)
{
	GPBCON = (0x1<<10) | (0x1<<12) | (0x1<<14) | (0x1<<16);
	GPBDAT = 0xF<<5;
}

/* num must be 1,2,3,4 */
void leds_on(int num)
{
	GPBDAT &= ~(1<<(num+4));
}

/* num must be 1,2,3,4 */
void leds_off(int num)
{
	GPBDAT |= (1<<(num+4));
}
/* ###################### End of leds ########################### */


/* ###################### Start of keys ######################### */
void keys_init(void)
{
	GPGCON = (0x0<<0) | (0x0<<6) | (0x0<<10) | (0x0<<12) | (0x0<<14) | (0x0<<22);
}

void keys_leds_ops(void)
{
	while (1) {
		if (GPGDAT & (1<<0))
			leds_off(1);
		else 
			leds_on(1);

		if (GPGDAT & (1<<3))
			leds_off(2);
		else 
			leds_on(2);
		
		if (GPGDAT & (1<<5))
			leds_off(3);
		else 
			leds_on(3);

		if (GPGDAT & (1<<6))
			leds_off(4);
		else 
			leds_on(4);

		if (GPGDAT & (1<<7)) {
			leds_off(1);
			leds_off(3);
		}else { 
			leds_on(1);
			leds_on(3);
		}

		if (GPGDAT & (1<<11)) {
			leds_off(2);
			leds_off(4);
		}else { 
			leds_on(2);
			leds_on(4);
		}
	}
}
/* ###################### End of keys ########################### */

int keys_leds_main(void)
{
	/* leds initialization */
	leds_init();

	/* keys initialization */
	keys_init();
	
	/* keys_leds_ops */
	keys_leds_ops();

	return 0;
}


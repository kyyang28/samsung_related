

extern void leds_init(void);
extern void key_int_init(void);

int key_int_main(void)
{
	/* leds initialization */
	leds_init();
	
	/* key interrupt initialization */
	key_int_init();
	
	while (1);
	
	return 0;
}


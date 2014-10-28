
/* LCD operations */
extern void lcd_init(void);
extern void lcd_clear_screen(unsigned int color);
extern void lcd_display_bmp(unsigned int bmp_addr);

/* Nandflash operations */
extern void nand_read_ll(unsigned int nand_addr, unsigned char *sdram_addr, unsigned int len);  

static void delay(void)
{
	volatile int i;
	for (i = 0; i < 600000; i++);
}

int lcd_main(void)
{
	lcd_init();

	while (1) {
		/* Clear the screen to "black" */
		lcd_clear_screen(0x0);
		delay();
		
		/* Clear the screen to "red" */
		lcd_clear_screen(0xff0000);
		delay();
		
		/* Clear the screen to "green" */
		lcd_clear_screen(0x00ff00);
		delay();
		
		/* Clear the screen to "blue" */
		lcd_clear_screen(0x0000ff);
		delay();

		/* Clear the screen to "white" */
		lcd_clear_screen(0xffffff);
		delay();

		nand_read_ll(0x20000, (unsigned char *)0x32000000, 0x60000);
		lcd_display_bmp(0x32000000);
		delay();
		delay();
	}

	return 0;
}


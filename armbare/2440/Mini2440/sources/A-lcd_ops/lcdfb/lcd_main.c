
#include "lcd_ops.h"

int lcd_main(void)
{
	/* Lcd initialization */
	lcd_init_ll();

	/* Testing the LCD program */
	lcd_test_ll();	

	while (1);
	return 0;
}


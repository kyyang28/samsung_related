
#include "stdio.h"
#include "uart_ops.h"
#include "lcd_ops.h"

#define WIDTHEIGHT	800
#define HEIGHT	480

int lcd_main(void)
{
	int ch = 0;

	/* Uart init */
	uart_init_ll();

	/* Lcd init */
	lcd_init_ll();

	while (1) {
		printf("\r\n###############LCD Test##############\r\n");
		printf("[1] lcd_clear_screen\r\n");
		printf("[2] lcd_draw_cross\r\n");
		printf("[3] lcd_draw_hline\r\n");
		printf("[4] lcd_draw_vline\r\n");
		printf("Enter your choice:");
		ch = uart_getc();
		printf("%c\r\n", ch);
		switch(ch) {
			case '1':
				lcd_clear_screen(0x000000);										/* Black */
				break;

			case '2':
				lcd_draw_cross(50, 50, 20, 0x0000ff);							/* Blue */
				break;

			case '3':
				lcd_draw_hline(HEIGHT/2, 100, WIDTHEIGHT-100, 0xff0000);		/* Red */
				break;

			case '4':
				lcd_draw_vline(WIDTHEIGHT/2, 50, HEIGHT-50, 0x00ff00);			/* Green */
				break;

			default:
				break;
		}	
	}
	
	return 0;
}


/**************************************************************
The initial and control for 16Bpp TFT LCD
**************************************************************/

#include "s3c2440-regs.h"
#include "lcd_ops.h"

#define MASK(n)				((n) & 0x1fffff)	// To get lower 21bits

volatile unsigned short LCD_BUFFER[SCR_YSIZE_TFT][SCR_XSIZE_TFT];

static void lcd_gpio_init(void)
{
	/* Setup the GPCCON and GPDCON registers */
	GPCCON = 0xaaaaaaaa;
	GPDCON = 0xaaaaaaaa;
}

static void lcd_controller_init(void)
{
	LCDCON1	= (CLKVAL_TFT<<8)|(MVAL_USED<<7)|(3<<5)|(12<<1)|0; // TFT LCD panel,12bpp TFT,ENVID=off
	LCDCON2	= (VBPD<<24)|(LINEVAL_TFT<<14)|(VFPD<<6)|(VSPW);
	LCDCON3	= (HBPD<<19)|(HOZVAL_TFT<<8)|(HFPD);
	LCDCON4	= (HSPW);
	LCDCON5 = (0<<12) | (1<<11) | (1<<10) | (1<<9) | (1<<8) | (1<<6) | (1<<3) |(BSWP<<1) | (HWSWP);

	LCDSADDR1 = (((u32)LCD_BUFFER>>22)<<21)|MASK((u32)LCD_BUFFER>>1);
	LCDSADDR2 = MASK( ((u32)LCD_BUFFER+(SCR_XSIZE_TFT*LCD_YSIZE_TFT*2))>>1 );
	LCDSADDR3 = (((SCR_XSIZE_TFT-LCD_XSIZE_TFT)/1)<<11)|(LCD_XSIZE_TFT/1);
	LCDINTMSK |= (3); // MASK LCD Sub Interrupt
	TCONSEL &= (~7) ;     // Disable LPC3480
	TPAL = 0; // Disable Temp Palette
}

static void lcd_enable(void)
{
	GPGCON |= (0x03<<8);	/* LCD_PWREN */
	LCDCON5 |= (0<<5);		/* LCD_PWREN normal */
	LCDCON5 |= (1<<3);		/* Enable PWREN signal */
	LCDCON1 |= (1<<0);		/* Enable the video output and the LCD control signal */
}

void lcd_init_ll(void)
{
	lcd_gpio_init();
	lcd_controller_init();
	lcd_enable();
}

/**************************************************************
TFT LCD Single Pixel Display
**************************************************************/
void lcd_draw_pixel(u32 x, u32 y, u32 c)
{
	if ((x < SCR_XSIZE_TFT) && (y < SCR_YSIZE_TFT))
		LCD_BUFFER[(y)][(x)] = c;
}

/**************************************************************
TFT LCD Fill The Whole Screen with a Specific Color
**************************************************************/
void lcd_clear_screen(u32 c)
{
	unsigned int x, y;
		
    for (y = 0; y < SCR_YSIZE_TFT; y++) {
    	for(x = 0; x < SCR_XSIZE_TFT; x++)
			LCD_BUFFER[y][x] = c;
    }
}

/**************************************************************
Display a BMP Picture on the screen
**************************************************************/
void lcd_display_bmp(int x0, int y0, int h, int l, unsigned char bmp[])
{
	int x, y;
	u32 c;
	int p = 0;
	
    for (y = y0; y < l; y++) {
    	for (x = x0; x < h; x++) {
    		c = bmp[p+1] | (bmp[p]<<8);

			if (((x0+x) < SCR_XSIZE_TFT) && ((y0+y) < SCR_YSIZE_TFT))
				LCD_BUFFER[y0+y][x0+x] = c;
			
    		p = p + 2 ;
    	}
    }
}

static void delay(void)
{
	volatile int i;
	for (i = 0; i < 1200000; i++);
}

void lcd_test_ll( void ) 
{
	while (1) {
		lcd_clear_screen( (0x00<<11) | (0x00<<5) | (0x00)  )  ;		//clear screen to black
		delay();

		lcd_clear_screen( (0x1f<<11) | (0x3f<<5) | (0x1f)  )  ;		//clear screen to white
		delay();
		
		lcd_clear_screen( (0x1f<<11) | (0x00<<5) | (0x00)  )  ;		//clear screen to red
		delay();
		
		lcd_clear_screen( (0x00<<11) | (0x3f<<5) | (0x00)  )  ;		//clear screen to green
		delay();
		
		lcd_clear_screen( (0x00<<11) | (0x00<<5) | (0x1f)  )  ;		//clear screen to blue
		delay();
		
		lcd_display_bmp(0, 0, 240, 320, LCD_FOURLEAF);
		delay();
		
		lcd_display_bmp(0, 0, 240, 320, LCD_FLOWER);
		delay();
	}
}


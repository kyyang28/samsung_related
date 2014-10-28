
#include "stdio.h"
#include "lcd_ops.h"

#define HSPW			(20 - 1)	/* Min. = 1; Max. = 40; so, Typ. = 20 */
#define HBPD			(26 - 1)	/* HBPD = Horizontal back porch = HS Blanking - HSPW = 46 - 20 = 26 */
#define HFPD			(210 - 1)	/* HFPD = Horizontal front porch = 210 */
#define VSPW			(10 - 1)	/* Min. = 1; Max. = 20; so, Typ. = 10 */
#define VBPD			(13 - 1)	/* VBPD = Vertical back porch = 23 - 10 = 13 */
#define VFPD			(22 - 1)	/* VFPD = Vertical front porch = 22 */

#define FB_ADDR			(0x23000000)
#define ROW				(480)
#define COL				(800)
#define HOZVAL			(COL - 1)
#define LINEVAL			(ROW - 1)

#define LeftTopX		(0)
#define LeftTopY		(0)
#define RightBotX		(799)
#define RightBotY		(479)

#if 0
/* LCD GPIO Init */
static void lcd_gpio_init_ll(void)
{
	GPF0CON = 0x22222222;
	GPF1CON = 0x22222222;
	GPF2CON = 0x22222222;
	GPF3CON = 0x00002222;
	return;
}
#endif

int lcd_init_ll(void)
{
	/* Lcd gpio init */
	//lcd_gpio_init_ll();
	GPF0CON = 0x22222222;
	GPF1CON = 0x22222222;
	GPF2CON = 0x22222222;
	GPF3CON = 0x00002222;

	/* Lcd backlight enable */
	//lcd_backlight_enable();
	GPD0CON &= ~(0xf<<4);
	GPD0CON |= (1<<4);
	GPD0DAT |= (1<<1);

	/* 10: RGB=FIMD I80=FIMD ITU=FIMD */
	//setup_lcd_display_control();
	DISPLAY_CONTROL = 2<<0;

	/* Setup display controller */
	//setup_display_controller();
	VIDCON0 &= ~((3<<26) | (1<<18) | (1<<2));

	/* Enable display controller signal */
	VIDCON0 |= ((1<<0)|(1<<1));

	/* Setup the CLKVAL_F */
	VIDCON0 |= (14<<6 | 1<<4);

	VIDCON1 |= 1<<5 | 1<<6;

	VIDTCON0 = VBPD<<16 | VFPD<<8 | VSPW<<0;
	VIDTCON1 = HBPD<<16 | HFPD<<8 | HSPW<<0;

	VIDTCON2 = (LINEVAL << 11) | (HOZVAL << 0);

	WINCON0 |= 1<<0;
	WINCON0 &= ~(0xf << 2);
	WINCON0 |= (0xB<<2) | (1<<15);

	VIDOSD0A = (LeftTopX<<11) | (LeftTopY << 0);
	VIDOSD0B = (RightBotX<<11) | (RightBotY << 0);
	VIDOSD0C = (LINEVAL + 1) * (HOZVAL + 1);

	VIDW00ADD0B0 = FB_ADDR;
	VIDW00ADD1B0 = (((HOZVAL + 1)*4 + 0) * (LINEVAL + 1)) & (0xffffff);

	SHADOWCON = 0x1;

	return 0;
}

void lcd_draw_pixel(int row, int col, int color)
{
	unsigned long * pixel = (unsigned long  *)FB_ADDR;
	*(pixel + row * COL + col) = color;
}

void lcd_clear_screen(int color)
{
	int i, j;

	for (i = 0; i < ROW; i++)
		for (j = 0; j < COL; j++)
			lcd_draw_pixel(i, j, color);
}

void lcd_draw_hline(int row, int col1, int col2, int color)
{
	int j;

	// 描第row行，第j列
	for (j = col1; j <= col2; j++)
		lcd_draw_pixel(row, j, color);
}

void lcd_draw_vline(int col, int row1, int row2, int color)
{
	int i;
	// 描第i行，第col列
	for (i = row1; i <= row2; i++)
		lcd_draw_pixel(i, col, color);
}

void lcd_draw_cross(int row, int col, int halflen, int color)
{
	lcd_draw_hline(row, col-halflen, col+halflen, color);
	lcd_draw_vline(col, row-halflen, row+halflen, color);
}


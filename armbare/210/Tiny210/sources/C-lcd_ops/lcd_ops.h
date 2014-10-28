#ifndef __LCD_OPS_H
#define __LCD_OPS_H

#define GPF0CON							(*(volatile unsigned long *)0xE0200120)
#define GPF1CON							(*(volatile unsigned long *)0xE0200140)
#define GPF2CON							(*(volatile unsigned long *)0xE0200160)
#define GPF3CON							(*(volatile unsigned long *)0xE0200180)

#define GPD0CON							(*(volatile unsigned long *)0xE02000A0)
#define GPD0DAT							(*(volatile unsigned long *)0xE02000A4)

#define DISPLAY_CONTROL					(*(volatile unsigned long *)0xe0107008)

#define VIDCON0							(*(volatile unsigned long *)0xF8000000)
#define VIDCON1							(*(volatile unsigned long *)0xF8000004)
#define VIDTCON2						(*(volatile unsigned long *)0xF8000018)
#define WINCON0							(*(volatile unsigned long *)0xF8000020)
#define WINCON2							(*(volatile unsigned long *)0xF8000028)
#define SHADOWCON						(*(volatile unsigned long *)0xF8000034)
#define VIDOSD0A						(*(volatile unsigned long *)0xF8000040)
#define VIDOSD0B						(*(volatile unsigned long *)0xF8000044)
#define VIDOSD0C						(*(volatile unsigned long *)0xF8000048)

#define VIDW00ADD0B0					(*(volatile unsigned long *)0xF80000A0)
#define VIDW00ADD1B0					(*(volatile unsigned long *)0xF80000D0)

#define VIDTCON0						(*(volatile unsigned long *)0xF8000010)
#define VIDTCON1						(*(volatile unsigned long *)0xF8000014)

int lcd_init_ll(void);
void lcd_draw_pixel(int row, int col, int color);
void lcd_clear_screen(int color);
void lcd_draw_hline(int row, int col1, int col2, int color);
void lcd_draw_vline(int col, int row1, int row2, int color);
void lcd_draw_cross(int row, int col, int halflen, int color);

#endif


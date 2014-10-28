/*
*********************************************************************************************************
*                                         uC/OS-II ON MINI2440 BOARD
                                                ARM920T Port
*                                             ADS v1.2 Compiler                                                                                     
*                               (c) Copyright 2011,ZhenGuo Yao,ChengDu,Uestc
*                                           All Rights Reserved
*
* File : lcd.h
* By   : ZhenGuoYao
*********************************************************************************************************
*/
#ifndef LCD_H
#define LCD_H

#define LCD_WIDTH 	240
#define LCD_HEIGHT 	320
#define LCD_PIXCLOCK 	4

#define LCD_RIGHT_MARGIN 	25
#define LCD_LEFT_MARGIN 	0
#define LCD_HSYNC_LEN 		4

#define LCD_UPPER_MARGIN 	1
#define LCD_LOWER_MARGIN 	4
#define LCD_VSYNC_LEN 		1
#define LCD_CON5 		( (1 << 11)| (1<<0) | (1 << 8) | (1 << 6) | (1 << 9) | ( 1<< 10))

  /* color */
#define  clWhite	0xFFFF //白色
#define  clBlack	0x0000 //黑色
#define  clDRed 	0x8000 //浅红色
#define  clLRed 	0xF800 //深红色
#define  clDMagenta	0x8010 //暗紫色
#define  clLMagenta	0xF81F //亮紫色
#define  clGreen	0x07E0 //绿色
#define  clDBlue	0x0010 //暗蓝色
#define  clLBlue	0x001F //亮蓝色
#define  clDCyan	0x0410 //暗青色
#define  clLCyan	0x07FF //亮青色
#define  clDYellow	0x8400 //暗黄色
#define  clLYellow	0xFFE0 //亮黄色
#define  clDGray	0x8410 //暗灰色
#define  clLGray	0xF79E //亮灰色
#define  clLArgent	0xCE79 //亮银色

#define LCD_TFT_XSIZE 	LCD_WIDTH	
#define LCD_TFT_YSIZE 	LCD_HEIGHT

#define SCR_XSIZE_TFT 	2*LCD_WIDTH
#define SCR_YSIZE_TFT 	2*LCD_HEIGHT

#define LCD_OFFSIZE    SCR_XSIZE_TFT-LCD_TFT_XSIZE 

void Lcd_T35_Init(void);

extern unsigned char shift480_640[];

void Lcd_ClearScr(unsigned short c);
void PutPixel(unsigned int x,unsigned int y, unsigned short c);
void GUI_Point(unsigned int x, unsigned int y, unsigned short c );
void Lcd_MoveViewPort(int vx,int vy);

void Glib_Rectangle(int x1,int y1,int x2,int y2,int color);
void Glib_FilledRectangle(int x1,int y1,int x2,int y2,int color);
void Glib_Line(int x1,int y1,int x2,int y2,int color);
void Paint_Bmp(int x0,int y0,int h,int l,unsigned char bm222p[]);
void drawCross(unsigned int x,unsigned int y,unsigned int color);

//================================
// 字体输出
//================================
void Lcd_PutASCII(unsigned int x,unsigned int y,unsigned char ch,unsigned int c,unsigned int bk_c,unsigned int st); 
void Lcd_PutHZ(unsigned int x,unsigned int y,unsigned short int QW,unsigned int c,unsigned int bk_c,unsigned int st);  
void Lcd_printf(unsigned int x,unsigned int y,unsigned int c,unsigned int bk_c,unsigned int st,char *inputs);

#endif



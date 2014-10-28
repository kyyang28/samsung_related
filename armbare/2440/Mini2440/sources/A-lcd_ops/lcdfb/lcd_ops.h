#ifndef __LCD_OPS_H
#define __LCD_OPS_H

/**************************************************************
The initial and control for TFT LCD
**************************************************************/
#define MVAL		(13)
#define MVAL_USED 	(0)			//0=each frame   1=rate by MVAL
#define INVVDEN		(1)			//0=normal       1=inverted
#define BSWP		(0)			//Byte swap control
#define HWSWP		(1)			//Half word swap control
#define PNRMODE		(3)		
#define BPPMODE		(12)

//TFT_SIZE
#define LCD_XSIZE_TFT 	(240)	
#define LCD_YSIZE_TFT 	(320)

#define SCR_XSIZE_TFT 	(240)
#define SCR_YSIZE_TFT 	(320)

//Timing parameter for 3.5' LCD
#define VBPD 		(1)			
#define VFPD 		(15)	
#define VSPW 		(1)	

#define HBPD 		(17)	
#define HFPD 		(9)		
#define HSPW 		(5)		

#define CLKVAL_TFT 	(6) 	

#define HOZVAL_TFT	(LCD_XSIZE_TFT-1)
#define LINEVAL_TFT	(LCD_YSIZE_TFT-1)

void lcd_init_ll(void);

void lcd_test_ll( void ) ;

extern unsigned char LCD_FOURLEAF[];

extern unsigned char LCD_FLOWER[];

#endif


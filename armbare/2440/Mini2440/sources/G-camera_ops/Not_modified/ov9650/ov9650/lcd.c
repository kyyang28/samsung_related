/*
*********************************************************************************************************
*                                         uC/OS-II ON MINI2440 BOARD
                                                ARM920T Port
*                                             ADS v1.2 Compiler                                                                                     
*                               (c) Copyright 2011,ZhenGuo Yao,ChengDu,Uestc
*                                           All Rights Reserved
*
* File : lcd.c
* By   : ZhenGuo Yao
*********************************************************************************************************
*/


#include "./include/2440addr.h"
#include "./include/2440lib.h"
#include "./include/lcd.h"

extern unsigned char __CHS[];
extern unsigned char __VGA[];

//volatile static
unsigned short LCD_BUFFER[SCR_YSIZE_TFT][SCR_XSIZE_TFT];

/**************************************************************
320��240 16Bpp TFT LCD����ģ���ʼ��
**************************************************************/
static void Lcd_PowerEnable(int invpwren,int pwren);

static void Lcd_Init(void)
{

#define	M5D(n)	((n)&0x1fffff)   ///��ǰ11λ����
#define LCD_ADDR ((unsigned int)LCD_BUFFER)
	//����ѡ��LCD���ͣ���������ʱ�ӣ�ʹ��LCD�ź����
	// VCLK=HCLK/(4+1)*2=10MHZ(HCLK=100MHZ),TFT LCD, 16BPP(64K) 
	rLCDCON1 = (LCD_PIXCLOCK << 8) | (3 <<  5) | (12 << 1);
	//��ֱ������ƼĴ�������
	//VBPD=LCD_UPPER_MARGIN=1,HEIGHT=LCD_HEIGHT=320,VFPD=LCD_LOWER_MARGIN=4,VSPW=LCD_VSYNC_LEN=1
	rLCDCON2 = (LCD_UPPER_MARGIN << 24) | ((LCD_HEIGHT - 1) << 14) | (LCD_LOWER_MARGIN << 6) | (LCD_VSYNC_LEN << 0);
	//��ֱ������ƼĴ�������
   	//HBPD=LCD_RIGHT_MARGIN=25,WIDTH=LCD_WIDTH=240,HFPD=LCD_LEFT_MARGIN=0
   	rLCDCON3 = (LCD_RIGHT_MARGIN << 19) | ((LCD_WIDTH  - 1) <<  8) | (LCD_LEFT_MARGIN << 0);
   	//HSPW=LCD_HSYNC_LEN=4
   	//HSYNC ������
   	rLCDCON4 = (13 <<  8) | (LCD_HSYNC_LEN << 0);

#if !defined(LCD_CON5)
#    define LCD_CON5 ((1<<11) | (1 << 9) | (1 << 8) | (1 << 3) | (1 << 0))
#endif
	//�������ø��������źŵļ���
	rLCDCON5   =  LCD_CON5;
    
	//��ʼ��֡�ڴ��ַ
	rLCDSADDR1 = ((LCD_ADDR >> 22) << 21) | ((M5D(LCD_ADDR >> 1)) <<  0);
	rLCDSADDR2 = M5D((LCD_ADDR + (LCD_WIDTH+LCD_OFFSIZE) * LCD_HEIGHT * 2) >> 1);
	rLCDSADDR3 = (LCD_OFFSIZE << 11) | LCD_WIDTH;        

	rLCDINTMSK |= 3;
  	rTCONSEL   &= (~7);
 
	//��ɫ��Ĵ���
   	rTPAL     = 0x0;
   	rTCONSEL &= ~((1<<4) | 1);       
}

/**************************************************************
LCD��Ƶ�Ϳ����ź��������ֹͣ��1������Ƶ���
**************************************************************/
static void Lcd_EnvidOnOff(int onoff)
{
    if(onoff==1)
	rLCDCON1|=1; // ENVID=ON
    else
	rLCDCON1 =rLCDCON1 & 0x3fffe; // ENVID Off
}

/**************************************************************
320��240 16Bpp TFT LCD ��Դ��������ʹ��
**************************************************************/
static void Lcd_PowerEnable(int invpwren,int pwren)
{
	//GPG4 is setted as LCD_PWREN
	rGPGUP = rGPGUP & ((~(1<<4)) | (1<<4)); // Pull-up disable
	rGPGCON = rGPGCON & ((~(3<<8)) | (3<<8)); //GPG4=LCD_PWREN
	rGPGDAT = rGPGDAT | (1<<4);
	//invpwren=pwren;
	//Enable LCD POWER ENABLE Function
	rLCDCON5 = rLCDCON5 & (~(1<<3)) | (pwren<<3);   // PWREN
	rLCDCON5 = rLCDCON5 & ((~(1<<5)) | (invpwren<<5));   // INVPWREN
}

/**************************************************************
320��240 16Bpp TFT LCD �ƶ��ӿ�
**************************************************************/
void Lcd_MoveViewPort(int vx,int vy)
{
    unsigned int addr;

    disable_irq();     //���ж�
	
    if((rLCDCON1>>18)!=0) // ������һ֡ͼ������һ���иı�֡�׵�ַ
    {    
        addr=(unsigned int)LCD_BUFFER+(vx*2)+vy*(SCR_XSIZE_TFT*2);
	    rLCDSADDR1= ( (addr>>22)<<21 ) | M5D(addr>>1);
	    rLCDSADDR2= M5D(((addr+(SCR_XSIZE_TFT*LCD_TFT_YSIZE*2))>>1));
	}
    enable_irq();    //���ж�
}    



/**************************************************************
320��240 16Bpp TFT LCD�������ص���ʾ�������
**************************************************************/
void PutPixel(unsigned int x,unsigned int y, unsigned short c )
{
	if ( (x < SCR_XSIZE_TFT) && (y < SCR_YSIZE_TFT) )
		LCD_BUFFER[(y)][(x)] = c;
}
 
void GUI_Point(unsigned int x,unsigned int y, unsigned short c )
{
	if ( (x < SCR_XSIZE_TFT) && (y < SCR_YSIZE_TFT) )
		LCD_BUFFER[(y)][(x)] = c;
}

/**************************************************************
320��240 16Bpp TFT LCDȫ������ض���ɫ��Ԫ������
**************************************************************/
void Lcd_ClearScr( unsigned short c)
{
	unsigned int x,y ;
		
    for( y = 0 ; y < SCR_YSIZE_TFT ; y++ )
    {
    	for( x = 0 ; x < SCR_XSIZE_TFT ; x++ )
    	{
			LCD_BUFFER[y][x] = c ;
    	}
    }
}

/**************************************************************
LCD��Ļ��ʾ��ֱ��ת
// LCD display is flipped vertically
// But, think the algorithm by mathematics point.
//   3I2
//   4 I 1
//  --+--   <-8 octants  mathematical cordinate
//   5 I 8
//   6I7
**************************************************************/
 void Glib_Line(int x1,int y1,int x2,int y2,int color)
{
	int dx,dy,e;
	dx=x2-x1; 
	dy=y2-y1;
    
	if(dx>=0)
	{
		if(dy >= 0) // dy>=0
		{
			if(dx>=dy) // 1/8 octant
			{
				e=dy-dx/2;
				while(x1<=x2)
				{
					PutPixel(x1,y1,color);
					if(e>0){y1+=1;e-=dx;}	
					x1+=1;
					e+=dy;
				}
			}
			else		// 2/8 octant
			{
				e=dx-dy/2;
				while(y1<=y2)
				{
					PutPixel(x1,y1,color);
					if(e>0){x1+=1;e-=dy;}	
					y1+=1;
					e+=dx;
				}
			}
		}
		else		   // dy<0
		{
			dy=-dy;   // dy=abs(dy)

			if(dx>=dy) // 8/8 octant
			{
				e=dy-dx/2;
				while(x1<=x2)
				{
					PutPixel(x1,y1,color);
					if(e>0){y1-=1;e-=dx;}	
					x1+=1;
					e+=dy;
				}
			}
			else		// 7/8 octant
			{
				e=dx-dy/2;
				while(y1>=y2)
				{
					PutPixel(x1,y1,color);
					if(e>0){x1+=1;e-=dy;}	
					y1-=1;
					e+=dx;
				}
			}
		}	
	}
	else //dx<0
	{
		dx=-dx;		//dx=abs(dx)
		if(dy >= 0) // dy>=0
		{
			if(dx>=dy) // 4/8 octant
			{
				e=dy-dx/2;
				while(x1>=x2)
				{
					PutPixel(x1,y1,color);
					if(e>0){y1+=1;e-=dx;}	
					x1-=1;
					e+=dy;
				}
			}
			else		// 3/8 octant
			{
				e=dx-dy/2;
				while(y1<=y2)
				{
					PutPixel(x1,y1,color);
					if(e>0){x1-=1;e-=dy;}	
					y1+=1;
					e+=dx;
				}
			}
		}
		else		   // dy<0
		{
			dy=-dy;   // dy=abs(dy)

			if(dx>=dy) // 5/8 octant
			{
				e=dy-dx/2;
				while(x1>=x2)
				{
					PutPixel(x1,y1,color);
					if(e>0){y1-=1;e-=dx;}	
					x1-=1;
					e+=dy;
				}
			}
			else		// 6/8 octant
			{
				e=dx-dy/2;
				while(y1>=y2)
				{
					PutPixel(x1,y1,color);
					if(e>0){x1-=1;e-=dy;}	
					y1-=1;
					e+=dx;
				}
			}
		}	
	}
}

/**************************************************************
��LCD��Ļ�ϻ�һ������
**************************************************************/
void Glib_Rectangle(int x1,int y1,int x2,int y2,int color)
{
    Glib_Line(x1,y1,x2,y1,color);
    Glib_Line(x2,y1,x2,y2,color);
    Glib_Line(x1,y2,x2,y2,color);
    Glib_Line(x1,y1,x1,y2,color);
}

/**************************************************************
��LCD��Ļ������ɫ���һ������
**************************************************************/
void Glib_FilledRectangle(int x1,int y1,int x2,int y2,int color)
{
    int i;

    for(i=y1;i<=y2;i++)
	Glib_Line(x1,i,x2,i,color);
}

/**************************************************************
��LCD��Ļ��ָ������㻭һ��ָ����С��ͼƬ
**************************************************************/
void Paint_Bmp(int x0,int y0,int h,int l,unsigned char bmp[])
{
	int x,y;
	unsigned int c;
	int p = 0;
	
    for( y = y0 ; y < l ; y++ )
    {
    	for( x = x0 ; x < h ; x++ )
    	{
    		c = bmp[p+1] | (bmp[p]<<8);

			if ( ( (x0+x) < SCR_XSIZE_TFT) && ( (y0+y) < SCR_YSIZE_TFT) )
				LCD_BUFFER[y0+y][x0+x] = c ;
			
    		p = p + 2 ;
    	}
    }
}

/**************************************************************
��LCD��Ļ��ָ������㻭һ�� ʮ �� 
**************************************************************/
void drawCross(unsigned int x,unsigned int y,unsigned int color)
{
       unsigned int i;
       for (i = x - 10;i < x + 11; i++) {
           PutPixel(i, y, color);
       }
       for(i = y - 10; i < y + 11; i++) {
           PutPixel(x, i, color);
       } 
             
       for (i = x - 10; i <= x - 5; i++) {
           PutPixel(i, y - 10, color);
       }
       for (i = x + 5; i <= x + 10; i++) {
           PutPixel(i, y - 10, color);
       }
       for (i = x - 10; i <= x - 5; i++) {
           PutPixel(i, y + 10, color);
       }
       for (i = x + 5; i <= x + 10; i++) {
           PutPixel(i, y + 10, color);
       }
       for (i = y - 10; i<= y - 5; i++) {
           PutPixel(x + 10,i, color);
       }
       for (i = y +5; i<= y + 10; i++) {
           PutPixel(x + 10,i, color);
       }
       for (i = y - 10; i<= y - 5; i++) {
           PutPixel(x - 10,i, color);
       }
       for (i = y +5; i<= y + 10; i++) {
           PutPixel(x - 10,i, color);
       }         
}

/**************************************************************
��LCD��Ļ��ָ�������дASCII��
**************************************************************/
void Lcd_PutASCII(unsigned int x,unsigned int y,unsigned char ch,unsigned int c,unsigned int bk_c,unsigned int st)
{
	unsigned short int i,j;
	unsigned char *pZK,mask,buf;
	
	

	pZK = &__VGA[ch*16];
	for( i = 0 ; i < 16 ; i++ )
	{
		mask = 0x80;
		buf = pZK[i];
        for( j = 0 ; j < 8 ; j++ )
        {
            if( buf & mask )
            {
                PutPixel(x+j,y+i,c);
            }else
            {
                if( !st )
                {
                    PutPixel(x+j,y+i,bk_c);
                }
            }
            
            mask = mask >> 1;
        }
	}
}

/**************************************************************
��LCD��Ļ��ָ�������д����
**************************************************************/
void Lcd_PutHZ(unsigned int x,unsigned int y,unsigned short int QW,unsigned int c,unsigned int bk_c,unsigned int st)
{
	unsigned short int i,j;
	unsigned char *pZK,mask,buf;

	pZK = &__CHS[ (  ( (QW >> 8) - 1 )*94 + (QW & 0x00FF)- 1 )*32 ];
	for( i = 0 ; i < 16 ; i++ )
	{
		//��
		mask = 0x80;
        buf = pZK[i*2];
        for( j = 0 ; j < 8 ; j++ )
        {
            if( buf & mask )
            {
                PutPixel(x+j,y+i,c);
            }else
            {
                if( !st )
                {
                    PutPixel(x+j,y+i,bk_c);
                }
            }
            mask = mask >> 1;
        } 
        
		//��
		mask = 0x80;
        buf = pZK[i*2 + 1];
        for( j = 0 ; j < 8 ; j++ )
        {
            if( buf & mask )
            {
                PutPixel(x+j + 8,y+i,c);
            }else
            {
                if( !st )
                {
                    PutPixel(x+j + 8,y+i,bk_c);
                }
            }
            mask = mask >> 1;
        }                 
	}
}

//----------------------
void Lcd_printf(unsigned int x,unsigned int y,unsigned int c,unsigned int bk_c,unsigned int st,char *inputs)
{
 	char *pStr = inputs;
    unsigned int i = 0;

 	 
    while(*pStr != 0 )
	{
		switch(*pStr)
		{
			case '\n' :
				{
			
                    break;
				}

			default:
				{
					if( (*pStr > 0xA0) & (*(pStr+1) > 0xA0) )  //�������
                    {
                        Lcd_PutHZ( x , y , (*pStr - 0xA0)*0x0100 + *(pStr+1) - 0xA0 , c , bk_c , st);

                        pStr++;
                        i++;

                        x += 16;
                    }else               //Ӣ�����
                    {
                        Lcd_PutASCII( x , y , *pStr , c , bk_c , st );

                        x += 8;

                    }

                    break;
				}
		}
		
		pStr++;
        i++;		

        if( i > 256 ) break;
	}
   
}

void Lcd_T35_Init(void)
{
    Lcd_Init();
    Lcd_PowerEnable(0, 1);
    Lcd_EnvidOnOff(1);	
    Lcd_ClearScr(0xff);	
}


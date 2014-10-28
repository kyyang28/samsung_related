#include "./include/2440addr.h"
#include "./include/2440lib.h"
#include "./include/lcd.h"
#include "./include/ov9650.h"
#include "./include/ov9650_register.h"

 
void delay(int a)
{
       int k,w;
       for(k=0;k<a;k++)
              for(w = 0; w < 100; w++);
}
 
void __inline SCCB_start(void)
{
       CLOCK_HIGH();
       DATA_HIGH();
       delay(1);
       DATA_LOW();
       delay(1);
       CLOCK_LOW();
       delay(1);
}
void __inline SCCB_end(void)
{
       DATA_LOW();
       delay(1);
       CLOCK_HIGH();
       delay(1);
       DATA_HIGH();
       delay(1);
}
void __inline SCCB_sendbyte(unsigned char data)
{
       int i=0;
       for(i=0;i<8;i++)
       {
              if(data & 0x80)
                     DATA_HIGH();
              else
                     DATA_LOW();
                    
              delay(1);
              CLOCK_HIGH();
              delay(1);
              CLOCK_LOW();
              delay(1);
              DATA_LOW();
              delay(1);
             
              data <<= 1;
       }
      
       DATA_HIGH();
       delay(1);
       CLOCK_HIGH();
       delay(1);
       CLOCK_LOW();
       delay(1);
}
 
void __inline SCCB_receivebyte(unsigned char *data)
{
       int i=0;
       int svalue=0;
       int pvalue = 0;
      
       rGPECON = 1<<28;             
      
       for(i=7;i>=0;i--)
       {
              CLOCK_HIGH();
              delay(1);
              svalue = rGPEDAT>>15;
              CLOCK_LOW();
              delay(1);
              pvalue |= svalue <<i;
       }
      
       rGPECON =5<<28;              
      
     
       DATA_HIGH();
       delay(1);
       CLOCK_HIGH();
       delay(1);
       CLOCK_LOW();
       delay(1);
      
       *data = pvalue &0xff;   
}
 

void SCCB_senddata(unsigned char subaddr, unsigned char data)
{
       
       SCCB_start();                             //启动SCCB
       SCCB_sendbyte(0x60);                //OV9650设备从地址，写操作
       SCCB_sendbyte(subaddr);            //设备内存地址
       SCCB_sendbyte(data);                 //写数据字节
       SCCB_end();                              //结束SCCB
      
       mdelay(1);
}
unsigned char SCCB_receivedata(unsigned char subaddr)
{
       unsigned char temp;
      
       //2相写传输周期
       SCCB_start();                             //启动SCCB
       SCCB_sendbyte(0x60);               //OV9650设备从地址，写操作
       SCCB_sendbyte(subaddr);            //设备内存地址
       SCCB_end();                              //结束SCCB
      
       //2相读传输周期
       SCCB_start();                             //启动SCCB
       SCCB_sendbyte(0x61);                //OV9650设备从地址，读操作
       SCCB_receivebyte(&temp);         //读字节
       SCCB_end();                              //结束SCCB
      
       mdelay(1);
       return temp;   
} 
void config_ov9650(void)
{
       unsigned char temp;
       int i;
 
       //¶ÁÈ¡OV9650³§ÉÌID
       i = 1;
       while (i)
       {
              temp = SCCB_receivedata(0x1C);               //»òRd_SCCB (0x1C,&temp);
	      Uart_Printf("the product ID1 is %d",temp);
              if(temp == 0x7F)
                     i = 0;
       }
       i=1;
       while(i)
       {
              temp = SCCB_receivedata(0x1D);               //»òRd_SCCB (0x1D,&temp);
	      Uart_Printf("the product ID2 is %d",temp);
              if(temp==0xA2)
                     i=0;
       }
 
       //žŽÎ»Ëùæ??V9650ŒÄŽæÆ÷
       SCCB_senddata(0x12,0x80);               //»òWr_SCCB (0x12,0x80);
       delay(10000);
      
       //Åäç??OV9650ŒÄŽæÆ÷
       for(i=0;i<((sizeof(ov9650_register))/2);i++)
       {
              SCCB_senddata(ov9650_register[i][0],ov9650_register[i][1]);  
              Uart_Printf("config ov9650 regsiter %d value :%d\n",ov9650_register[i][0],ov9650_register[i][1]);
       }
}
void CalculateBurstSize(unsigned int hSize,unsigned int *mainBurstSize,unsigned int *remainedBurstSize)
{
       unsigned int tmp;
       tmp=(hSize/4)%16;
       switch(tmp) {
              case 0:
                     *mainBurstSize=16;
                     *remainedBurstSize=16;
                     break;
              case 4:
                     *mainBurstSize=16;
                     *remainedBurstSize=4;
                     break;
              case 8:
                     *mainBurstSize=16;
                     *remainedBurstSize=8;
                     break;
              default:
                     tmp=(hSize/4)%8;
                     switch(tmp) {
                            case 0:
                                   *mainBurstSize=8;
                                   *remainedBurstSize=8;
                                   break;
                            case 4:
                                   *mainBurstSize=8;
                                   *remainedBurstSize=4;
                            default:
                                   *mainBurstSize=4;
                                   tmp=(hSize/4)%4;
                                   *remainedBurstSize= (tmp) ? tmp: 4;
                                   break;
                     }
                     break;
       }                                        
}
 
//ŒÆËãÔ€Ëõ·Å±ÈÂÊŒ°ÒÆÎ»Á¿£¬ÓÃÓÚCICOSCPRERATIOŒÄŽæÆ÷
void CalculatePrescalerRatioShift(unsigned int SrcSize, unsigned int DstSize, unsigned int *ratio,unsigned int *shift)
{
       if(SrcSize>=64*DstSize) {
              //Uart_Printf("ERROR: out of the prescaler range: SrcSize/DstSize = %d(< 64)/n",SrcSize/DstSize);
              while(1);
       }
       else if(SrcSize>=32*DstSize) {
              *ratio=32;
              *shift=5;
       }
       else if(SrcSize>=16*DstSize) {
              *ratio=16;
              *shift=4;
       }
       else if(SrcSize>=8*DstSize) {
              *ratio=8;
              *shift=3;
       }
       else if(SrcSize>=4*DstSize) {
              *ratio=4;
              *shift=2;
       }
       else if(SrcSize>=2*DstSize) {
              *ratio=2;
              *shift=1;
       }
       else {
              *ratio=1;
              *shift=0;
       }        
}
 
//ÉãÏñœÓ¿Úå??ÊŒ»¯
//ÊäÈë²ÎÊý·Ö±ðÎªÔ€ÀÀÄ¿±ê¿íºÍžß£šŒŽLCD³ßŽç£©£¬ÒÔŒ°Ë®ÆœºÍå??Ö±Æ«ÒÆÁ¿
void CamInit(unsigned int PrDstWidth, unsigned int PrDstHeight, unsigned int WinHorOffset, unsigned int WinVerOffset)
{
       unsigned int WinOfsEn;
       unsigned int MainBurstSizeRGB, RemainedBurstSizeRGB;
       unsigned int H_Shift, V_Shift, PreHorRatio, PreVerRatio, MainHorRatio, MainVerRatio;
       unsigned int SrcWidth, SrcHeight;
       unsigned int ScaleUp_H_Pr, ScaleUp_V_Pr;
      
       //ÅÐ¶ÏÊÇ·ñÐèÒªÉèÖÃÆ«ÒÆÁ¿
       if(WinHorOffset==0 && WinVerOffset==0)
              WinOfsEn=0;
       else
              WinOfsEn=1;
 
       SrcWidth=640/*ÔŽË®Æœ³ßŽç*/-WinHorOffset*2;
       SrcHeight=480/*ÔŽŽ¹ç??³ßŽç*/-WinVerOffset*2;
 
       //ÅÐ¶Ï³ßŽçÊÇ·ÅŽó»¹ÊÇËõÐ¡
       if(SrcWidth>=PrDstWidth)
              ScaleUp_H_Pr=0;         //down
       else
              ScaleUp_H_Pr=1;         //up
 
       if(SrcHeight>=PrDstHeight)
              ScaleUp_V_Pr=0;  
       else
              ScaleUp_V_Pr=1;        
 
       rCIGCTRL |= (1<<26)|(0<<27);          //PCLKŒ«ÐÔ·Ž×ª£¬Íâ²¿æ????ŽŠÀíÆ÷ÊäÈë
       rCIWDOFST = (1<<30)|(0xf<<12);    //ÇåFIFOÒç³ö
       rCIWDOFST = 0;                //»ÖžŽÕý³£Ä£Êœ
       rCIWDOFST=(WinOfsEn<<31)|(WinHorOffset<<16)|(WinVerOffset);     //ÉèÖÃÆ«ÒÆÁ¿
       rCISRCFMT=(1<<31)|(0<<30)|(0<<29)|(640/*ÔŽË®Æœ³ßŽç*/<<16)|(0<<14)|(480/*ÔŽŽ¹Ö±³ßŽç*/);
      
       //ÉèÖÃÄÚŽæÊ×µØÖ·£¬ÒòÎªÊÇÖ±œÓÏÔÊŸ£¬ËùÒÔÉèÖÃÎªLCD»ºŽæÊý×éÊ×µØÖ·
       rCIPRCLRSA1 = (unsigned int)LCD_BUFFER;
       rCIPRCLRSA2 = (unsigned int)LCD_BUFFER;
       rCIPRCLRSA3 = (unsigned int)LCD_BUFFER;
       rCIPRCLRSA4 = (unsigned int)LCD_BUFFER;
      
       //ÉèÖÃÄ¿±ê³ßŽç£¬²¢ÇÒ²»œøÐÐŸµÏñºÍÐý×ªŽŠÀí
       rCIPRTRGFMT=(PrDstWidth<<16)|(0<<14)|(PrDstHeight);
 
       //ŒÆËãå??ÉèÖÃÍ»·¢³€¶È
       CalculateBurstSize(PrDstWidth*2, &MainBurstSizeRGB, &RemainedBurstSizeRGB);
       rCIPRCTRL=(MainBurstSizeRGB<<19)|(RemainedBurstSizeRGB<<14);
 
//ŒÆËãË®ÆœºÍŽ¹Ö±Ëõ·Åæ??ÂÊºÍÎ»ÒÆÁ¿£¬ÒÔŒ°Ö÷Ë®Æœ¡¢Ž¹Ö±±ÈÂÊ
       CalculatePrescalerRatioShift(SrcWidth, PrDstWidth, &PreHorRatio, &H_Shift);
       CalculatePrescalerRatioShift(SrcHeight, PrDstHeight, &PreVerRatio, &V_Shift);
       MainHorRatio=(SrcWidth<<8)/(PrDstWidth<<H_Shift);
       MainVerRatio=(SrcHeight<<8)/(PrDstHeight<<V_Shift);
 
       //ÉèÖÃËõ·ÅËùÐèµÄž÷Àà²ÎÊý
       rCIPRSCPRERATIO=((10-H_Shift-V_Shift)<<28)|(PreHorRatio<<16)|(PreVerRatio);   
       rCIPRSCPREDST=((SrcWidth/PreHorRatio)<<16)|(SrcHeight/PreVerRatio);
       rCIPRSCCTRL=(1<<31)|(0/*16λ���ɫ*/ <<30)|(ScaleUp_H_Pr<<29)|(ScaleUp_V_Pr<<28)|(MainHorRatio<<16)|(MainVerRatio);
      
       //ÉèÖÃÃæ»ý
       rCIPRTAREA= PrDstWidth*PrDstHeight;
}
 
 
void CamPortSet(void)
{
	rGPJCON = 0x2aaaaaa;
	rGPJDAT = 0;
	rGPJUP = 0;

        rGPEUP = 0xc000;               //������Ч
        rGPECON = 5<<28;             //GPE15ΪSIO_D��GPE14ΪSIO_C����Ϊ���
} 

void ov9650_test(void)
{
       int HOffset,VOffset;
 
       //��ʼ��UPLL�õ�ov9650��ʱ��
       rUPLLCON = (56<<12) | (2<<4) | 1;          //UPLL=96MHz
       rCLKDIVN |= (1<<3);                 //UCLK = UPLL/2=48MHz
       rCAMDIVN = (rCAMDIVN & ~(0xf))|(1<<4)|(2);           //����ʱ�ӽӿ�ʱ�ӷ�Ƶ
        
       //��������GPIOJ�ڵ����� 
       CamPortSet();

      
       //Ӳ����λ
       rGPJDAT |= 1<<12;
       delay(100);
       rGPJDAT &= ~(1<<12);
      
       //������λ
       rCIGCTRL |= (1<<31);
       delay(100);
       rCIGCTRL &= ~(1<<31);
       delay(100);
 
       rCIGCTRL |= (1<<30);
       delay(300);
       rCIGCTRL &= ~(1<<30);
       delay(20000);

       Delay(80);

       config_ov9650();          
     
       HOffset=0;
       VOffset=0;
      
       //³õÊŒ»¯ÉãÏñœÓ¿Ú
       CamInit(240,640,HOffset,VOffset);
 
      
       rCIPRSCCTRL|=(1<<15);                   //Ô€ÀÀËõ·Å¿ªÆô
       rCIIMGCPT =(1<<31)|(1<<29);          //Ô€ÀÀËõ·Å²¶×œÊ¹ÄÜ
 
 
       while(1);
}

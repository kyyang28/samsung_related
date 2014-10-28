
// *************************************************
// ����ѧARM9��ADS1.2��һ���ܺõ����̣�������̼��׶���
// ������̿����ÿ������ǵ�LED�ƺͷ�����������Ӳ���ĺû���
// ������CACHE�Գ��������ٶȵ�Ӱ�죬��������FCLK��Ƶ�ʡ�
// LED test
// ��Ϊ����������4k,��˸ó������ʹ��supervivi�Ĺ�
// �ܲ˵�"v"����"n"ֱ����д��nand flash������
// ע��: Ҫ��д��nand flash�����У������RO��ַ����Ϊ0x0
// ����֮�� - 2008.08.30
// http://www.arm9.net or http://www.arm123.net
//***************************************************
#include "def.h"
#include "option.h"
#include "2440addr.h"     
#include "2440lib.h"
#include "2440slib.h"      
//================================

void dely(U32 tt)
{
   U32 i;
   for(;tt>0;tt--)
   {
     for(i=0;i<10000;i++){}
   }
}
   

int Main(int argc, char **argv)
{
	int i;
	U8 key;
	U32 mpll_val=0;
	int data;
  
	mpll_val = (92<<12)|(1<<4)|(1);
	
	//init FCLK=400M, so change MPLL first
	ChangeMPllValue((mpll_val>>12)&0xff, (mpll_val>>4)&0x3f, mpll_val&3);
	ChangeClockDivider(key, 12);    

	//ChangeClockDivider(1,1);    // 1:2:4    FCLK:HCLK:PCLK
    // rCLKDIVN=0x4;    //  1:4:4
    //ChangeMPllValue(82,2,1);     //FCLK=135.0Mhz     
	//ChangeMPllValue(82,1,1);     //FCLK=180.0Mhz     
    //ChangeMPllValue(161,3,1);    //FCLK=202.8Mhz 
    //ChangeMPllValue(117,1,1);    //FCLK=250.0Mhz 
    //ChangeMPllValue(122,1,1);    //FCLK=260.0Mhz 
    //ChangeMPllValue(125,1,1);    //FCLK=266.0Mhz 
    //ChangeMPllValue(127,1,1);    //FCLK=270.0Mhz  
    
    //MMU_EnableICache();
    //MMU_EnableDCache();
    
    MMU_DisableICache();
    MMU_DisableDCache();


    rGPBCON = 0x155555;
   
   
   	data = 0x06;
   	while(1)
   	{
   		
   		rGPBDAT = (data<<5);
   		dely(120);
   		data =~data;
	}
   
   return 0;
}
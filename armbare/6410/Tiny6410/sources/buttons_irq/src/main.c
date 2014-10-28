/**********************************************************
*实验要求：   按键中断方式操作蜂鸣器发声。
*功能描述：   本实验代码针对tiny6410开发板，并兼容mini6410。
*           tiny6410的8个按键资源如下：
*             KEYINT1 -- GPN0 -- XEINT0
*             KEYINT2 -- GPN1 -- XEINT1
*             KEYINT3 -- GPN2 -- XEINT2
*             KEYINT4 -- GPN3 -- XEINT3
*             KEYINT5 -- GPN4 -- XEINT4
*             KEYINT6 -- GPN5 -- XEINT5
*             KEYINT7 -- GPL11-- XEINT
*             KEYINT8 -- GPL12-- XEINT5
*             以上EINT资源均是EINT Group 0的资源。
*             在程序中使用了KEYINT1、KEYINT3和KEYINT5，分别
*           设置为低电平中断、下降沿中断和双沿中断，在中断发
*           生后会由蜂鸣器发出一段固定频率的声音。       
*日    期：   2011-3-10
*作    者：   国嵌
**********************************************************/
#include "gpio.h"
#include "def.h"

#include "system.h"
#include "intc.h"

//给指定地址赋整数值
#define Outp32(addr, data)	(*(volatile u32 *)(addr) = (data))
//读出指定地址的值
#define Inp32(addr)			(*(volatile u32 *)(addr))
//GPIO
#define GPIO_BASE				(0x7F008000)
//oGPIO_REGS类型在 gpio.h 中定义
#define GPIO 	   (( volatile oGPIO_REGS *)GPIO_BASE)

//函数声明
void __irq Isr_Eint(void);

void delay(int times);
void BuzzerPortInit(void);
void BuzzerPlay(u32 count);
void KeyIntPortInit(u32 uKey, u32 uType);
void EintClrPend(u32 uEINT_No );
void EintDisMask(u32 uEINT_No );
void KeyEIntInit(void);

/*
 * 程序入口
 * */
int main(void)
{	
	SYSTEM_EnableVIC();
	SYSTEM_EnableIRQ();
	INTC_Init();
	
	BuzzerPortInit();	
	KeyEIntInit();
	
	while(1);   
}

/*
 * 延时函数
 * */
void delay(int times)
{
    int i;
    for(;times>0;times--)
      for(i=0;i<3000;i++);
}

/*
 *   初始化蜂鸣器端口
 * @ 这里仅仅是把对应端口GPF14初始化为Output，没有使用
 * PWM TOUT的特殊功能
 * */
void BuzzerPortInit(void)
{
	u32 uConValue;

	uConValue = GPIO->rGPIOFCON;   
	uConValue &= ~(0x3<<28);   
	uConValue |= 0x1<<28;
	GPIO->rGPIOFCON = uConValue;	
}

/*
 * 调节对应端口电平，使蜂鸣器发出一段声音
 * */
void BuzzerPlay(u32 count)
{
	while(count--)
	{
		GPIO->rGPIOFDAT |= 0x1<<14;
		delay(10);
		GPIO->rGPIOFDAT &= ~(0x1<<14);
		delay(10);		
	}	
}

/*
 * 按键中断处理函数，在其中使蜂鸣器发出一段声音
 * */
void __irq Isr_Eint(void)
{         
	//清除中断悬起位
	EintClrPend(0);
	EintClrPend(2);	
	EintClrPend(4);
	
	BuzzerPlay(3);
	//清除rVIC0ADDR，该寄存器按位记录哪个VIC0中断源曾发生了中断
	Outp32(rVIC0ADDR, 0);
    
}

/*
 *   初始化KeyInt对应的端口及EINT的类型
 * @uKey  -- Tiny6410上有8个按键，uKey对应按键1-6
 * @uType -- 中断类型
 *   注意按键(1,2)、(3,4)、(5,6)中断类型由于使用相同寄存器位
 * 设置，所以它们的中断类型两两一致。
 * */
void KeyIntPortInit(u32 uKey, u32 uType)
{
	u32 uConValue;
	if (uKey==0||uKey>6)
		return;
	//设置对应IO为特殊功能 -- EINT
	uConValue = GPIO->rGPIONCON;   
	uConValue &= ~(0x3<<((uKey-1)<<1));   
	uConValue |= 0x2<<((uKey-1)<<1);
	GPIO->rGPIONCON = uConValue;
	//设置对应IO为上下拉除能
	uConValue = GPIO->rGPIONPUD;   
	uConValue &= ~(0x3<<((uKey-1)<<1));   
	uConValue |= 0x0<<((uKey-1)<<1);
	GPIO->rGPIONPUD = uConValue;
	//设置对应EINT为@uType 类型中断
	uConValue = GPIO->rEINT0CON0;   
	uConValue &= ~(0x7<<(((uKey-1)/2)<<2));   
	uConValue |= uType<<(((uKey-1)/2)<<2);
	GPIO->rEINT0CON0 = uConValue;		
}

/*
 * 清除中断悬起位，方法是向对应位写1
 * */
void EintClrPend(u32 uEINT_No )
{
	GPIO->rEINT0PEND = 1<<uEINT_No;;	
}

/*
 * 除能Eint[27:0]中断屏蔽位 
 * */
void EintDisMask(u32 uEINT_No )
{
	u32 uConValue;
	uConValue = GPIO->rEINT0MASK;      
	uConValue &= ~(0x1<<uEINT_No);
	GPIO->rEINT0MASK = uConValue;	
}

/*
 *   初始化EInt，将EINT Group 0中的EINT0,1设置为Low_Level、
 * EINT2,3设置为Falling_Edge、EINT4,5设置为Both_Edge
 * */
void KeyEIntInit(void)
{
	u32 uConValue;
	//初始化各Key对应的端口
	KeyIntPortInit(1, Low_Level);
	KeyIntPortInit(3, Falling_Edge);
	KeyIntPortInit(5, Both_Edge);
	//清除中断悬起位
	EintClrPend(0);
	EintClrPend(2);	
	EintClrPend(4);	
	//向rVIC0VECTADDR中写入对应中断服务程序的地址
	Outp32(rVIC0VECTADDR, (unsigned)Isr_Eint);
	Outp32(rVIC0VECTADDR+4, (unsigned)Isr_Eint);
	//使能中断源:
	//	INT_EINT0: External interrupt 4 ~ 11
	//	INT_EINT1: External interrupt 0 ~ 3
	uConValue = Inp32(rVIC0INTENABLE);
	uConValue |= (1<<NUM_EINT0)|(1<<NUM_EINT1);
	Outp32(rVIC0INTENABLE, uConValue);
	//解除屏蔽
	EintDisMask(0);		
	EintDisMask(2);
	EintDisMask(4);	
}


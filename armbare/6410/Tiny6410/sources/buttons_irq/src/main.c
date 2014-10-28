/**********************************************************
*ʵ��Ҫ��   �����жϷ�ʽ����������������
*����������   ��ʵ��������tiny6410�����壬������mini6410��
*           tiny6410��8��������Դ���£�
*             KEYINT1 -- GPN0 -- XEINT0
*             KEYINT2 -- GPN1 -- XEINT1
*             KEYINT3 -- GPN2 -- XEINT2
*             KEYINT4 -- GPN3 -- XEINT3
*             KEYINT5 -- GPN4 -- XEINT4
*             KEYINT6 -- GPN5 -- XEINT5
*             KEYINT7 -- GPL11-- XEINT
*             KEYINT8 -- GPL12-- XEINT5
*             ����EINT��Դ����EINT Group 0����Դ��
*             �ڳ�����ʹ����KEYINT1��KEYINT3��KEYINT5���ֱ�
*           ����Ϊ�͵�ƽ�жϡ��½����жϺ�˫���жϣ����жϷ�
*           ������ɷ���������һ�ι̶�Ƶ�ʵ�������       
*��    �ڣ�   2011-3-10
*��    �ߣ�   ��Ƕ
**********************************************************/
#include "gpio.h"
#include "def.h"

#include "system.h"
#include "intc.h"

//��ָ����ַ������ֵ
#define Outp32(addr, data)	(*(volatile u32 *)(addr) = (data))
//����ָ����ַ��ֵ
#define Inp32(addr)			(*(volatile u32 *)(addr))
//GPIO
#define GPIO_BASE				(0x7F008000)
//oGPIO_REGS������ gpio.h �ж���
#define GPIO 	   (( volatile oGPIO_REGS *)GPIO_BASE)

//��������
void __irq Isr_Eint(void);

void delay(int times);
void BuzzerPortInit(void);
void BuzzerPlay(u32 count);
void KeyIntPortInit(u32 uKey, u32 uType);
void EintClrPend(u32 uEINT_No );
void EintDisMask(u32 uEINT_No );
void KeyEIntInit(void);

/*
 * �������
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
 * ��ʱ����
 * */
void delay(int times)
{
    int i;
    for(;times>0;times--)
      for(i=0;i<3000;i++);
}

/*
 *   ��ʼ���������˿�
 * @ ��������ǰѶ�Ӧ�˿�GPF14��ʼ��ΪOutput��û��ʹ��
 * PWM TOUT�����⹦��
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
 * ���ڶ�Ӧ�˿ڵ�ƽ��ʹ����������һ������
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
 * �����жϴ�������������ʹ����������һ������
 * */
void __irq Isr_Eint(void)
{         
	//����ж�����λ
	EintClrPend(0);
	EintClrPend(2);	
	EintClrPend(4);
	
	BuzzerPlay(3);
	//���rVIC0ADDR���üĴ�����λ��¼�ĸ�VIC0�ж�Դ���������ж�
	Outp32(rVIC0ADDR, 0);
    
}

/*
 *   ��ʼ��KeyInt��Ӧ�Ķ˿ڼ�EINT������
 * @uKey  -- Tiny6410����8��������uKey��Ӧ����1-6
 * @uType -- �ж�����
 *   ע�ⰴ��(1,2)��(3,4)��(5,6)�ж���������ʹ����ͬ�Ĵ���λ
 * ���ã��������ǵ��ж���������һ�¡�
 * */
void KeyIntPortInit(u32 uKey, u32 uType)
{
	u32 uConValue;
	if (uKey==0||uKey>6)
		return;
	//���ö�ӦIOΪ���⹦�� -- EINT
	uConValue = GPIO->rGPIONCON;   
	uConValue &= ~(0x3<<((uKey-1)<<1));   
	uConValue |= 0x2<<((uKey-1)<<1);
	GPIO->rGPIONCON = uConValue;
	//���ö�ӦIOΪ����������
	uConValue = GPIO->rGPIONPUD;   
	uConValue &= ~(0x3<<((uKey-1)<<1));   
	uConValue |= 0x0<<((uKey-1)<<1);
	GPIO->rGPIONPUD = uConValue;
	//���ö�ӦEINTΪ@uType �����ж�
	uConValue = GPIO->rEINT0CON0;   
	uConValue &= ~(0x7<<(((uKey-1)/2)<<2));   
	uConValue |= uType<<(((uKey-1)/2)<<2);
	GPIO->rEINT0CON0 = uConValue;		
}

/*
 * ����ж�����λ�����������Ӧλд1
 * */
void EintClrPend(u32 uEINT_No )
{
	GPIO->rEINT0PEND = 1<<uEINT_No;;	
}

/*
 * ����Eint[27:0]�ж�����λ 
 * */
void EintDisMask(u32 uEINT_No )
{
	u32 uConValue;
	uConValue = GPIO->rEINT0MASK;      
	uConValue &= ~(0x1<<uEINT_No);
	GPIO->rEINT0MASK = uConValue;	
}

/*
 *   ��ʼ��EInt����EINT Group 0�е�EINT0,1����ΪLow_Level��
 * EINT2,3����ΪFalling_Edge��EINT4,5����ΪBoth_Edge
 * */
void KeyEIntInit(void)
{
	u32 uConValue;
	//��ʼ����Key��Ӧ�Ķ˿�
	KeyIntPortInit(1, Low_Level);
	KeyIntPortInit(3, Falling_Edge);
	KeyIntPortInit(5, Both_Edge);
	//����ж�����λ
	EintClrPend(0);
	EintClrPend(2);	
	EintClrPend(4);	
	//��rVIC0VECTADDR��д���Ӧ�жϷ������ĵ�ַ
	Outp32(rVIC0VECTADDR, (unsigned)Isr_Eint);
	Outp32(rVIC0VECTADDR+4, (unsigned)Isr_Eint);
	//ʹ���ж�Դ:
	//	INT_EINT0: External interrupt 4 ~ 11
	//	INT_EINT1: External interrupt 0 ~ 3
	uConValue = Inp32(rVIC0INTENABLE);
	uConValue |= (1<<NUM_EINT0)|(1<<NUM_EINT1);
	Outp32(rVIC0INTENABLE, uConValue);
	//�������
	EintDisMask(0);		
	EintDisMask(2);
	EintDisMask(4);	
}


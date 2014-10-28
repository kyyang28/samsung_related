#include "clock.h"
#include "led.h"
#include "uart.h"
#include "lib.h"
#include "sdhc.h"

#define UBLOCKS                16   //�������ٿ�

unsigned int *Tx_buffer;	//128[word]*16[blk]=8192[byte]
unsigned int *Rx_buffer;	//128[word]*16[blk]=8192[byte]
volatile unsigned int block;   // ע��������

void Flush_Rx_buf(void)
{
	//-- Flushing Rx buffer 
	int i;

	Rx_buffer=(unsigned int *)0x21800000;

	for(i=0;i<2048;i++)	//128[word]*16[blk]=8192[byte]
	*(Rx_buffer+i)=0;
	wy_printf("End Rx buffer flush\n");
}

void TR_Buf_new(void)
{
	//-- Tx & Rx Buffer initialize
	int i, j;

	Tx_buffer=(unsigned int *)0x21000000;

	j=0;
	for(i=0;i<2048;i++)	//128[word]*16[blk]=8192[byte]
	*(Tx_buffer+i)=i+j;
	wy_printf("End Tx buffer flush\n");

	Flush_Rx_buf();
}

void View_Rx_buf()
{
	//-- Display Rx buffer 
	int i,error=0;

	Tx_buffer=(unsigned int *)0x21000000;
	Rx_buffer=(unsigned int *)0x21800000;

	wy_printf("Check Rx data\n");

	for(i=0;i<2048;i++) // 2048 = 8192/4
	{
		if(Rx_buffer[i] != Tx_buffer[i])
		{
			wy_printf("\nTx/Rx error\n"); 
			wy_printf("%d:Tx-0x%08x, Rx-0x%08x\n",i,Tx_buffer[i], Rx_buffer[i]);
			error=1;
			break;
		}
	}

	if(!error)
	{
		wy_printf("\nThe Tx_buffer is same to Rx_buffer!\n");
		wy_printf("SD CARD Write and Read test is OK!\n");
	}
}

int main(void)
{
	led_init(); /* ���ö�Ӧ�ܽ�Ϊ��� */
	clock_init(); /* ��ʼ��ʱ�� */
	uart_init(); /* ��ʼ��UART0 */

	wy_printf("\n*********************************************\n");
	wy_printf("                test sd card!!\n");
	wy_printf("*********************************************\n");

	SDHC_Init();

	wy_printf("\n\n\nStart SD Card Write and Read Test\n");
	TR_Buf_new(); /* ���ڳ�ʼ�������ڴ����� */

	/* д������ */
	SDHC_WriteBlocks(0x0,UBLOCKS,Tx_buffer); //����1:���������ݵĵ�ַ(��ΪĿ��) ����2:���ٿ� ����3:�ڴ��ַ(��ΪԴ)
	/* �������� */
	SDHC_ReadBlocks(0x0,UBLOCKS,Rx_buffer);
	
	View_Rx_buf(); /* ��ʾ���� */
	while (1);
	
	return 0;
}


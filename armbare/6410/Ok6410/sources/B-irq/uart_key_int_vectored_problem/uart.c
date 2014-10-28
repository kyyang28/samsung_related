
#include "s3c6410_regs.h"

extern void led_on(int num);
extern void led_off(int num);


void uart_tx_int_enable(void)
{
	UINTM0 &= ~(1<<2);	
}

void uart_tx_int_disable(void)
{
	UINTM0 |= (1<<2);	
}


void uart_init(void)
{
	GPACON &= ~0xff;
	GPACON |= 0x22;
	
	/* ULCON0 */
	ULCON0 = 0x3;  /* ����λ:8, �޽���, ֹͣλ: 1, 8n1 */
	UCON0  = 0x5 | (1<<9);  /* ʹ��UART���͡�����, tx interrupt request type = level */
#ifdef ENABLE_FIFO
	UFCON0 = 0x07 | (1<<6); /* FIFO enable, tx fifo trigger level = 16 bytes */	
#else
	UFCON0 = 0x00; /* FIFO disable */
#endif
	UMCON0 = 0;
	
	/* ������ */
	/* DIV_VAL = (PCLK / (bps x 16 ) ) - 1 
	 * bps = 57600
	 * DIV_VAL = (66500000 / (115200 x 16 ) ) - 1 
	 *         = 35.08
	 */
	UBRDIV0   = 35;

	/* x/16 = 0.08
	 * x = 1
	 */
	UDIVSLOT0 = 0x1;
	UINTM0 = 0xF;
}

unsigned char uart_getc(void)
{
#ifdef ENABLE_FIFO
	while (!(UFSTAT0 & 0x7f));
#else	
	while ((UTRSTAT0 & (1<<0)) == 0);
#endif

	return URXH0;
}


#define TX_BUF_LEN   2048
static unsigned char txbuf[2047];
static unsigned int r_idx = 0;
static unsigned int w_idx = 0;

static int isFull(void)
{
	if ((w_idx + 1) % TX_BUF_LEN == r_idx)
		return 1;
	else
		return 0;
}

static int isEmpty(void)
{
	return (w_idx == r_idx);
}

static int putData(unsigned char data)
{
	if (isFull())
		return -1;
	else
	{
		txbuf[w_idx] = data;
		w_idx = (w_idx + 1) % TX_BUF_LEN;
		return 0;
	}
}

static int getData(unsigned char *pdata)
{
	if (isEmpty())
	{
		return -1;
	}
	else
	{
		*pdata = txbuf[r_idx];
		r_idx = (r_idx + 1) % TX_BUF_LEN;
		return 0;
	}
}

void uart_putc(char c)
{
	putData(c);    /* �����ݷŵ���������ȥ */

	/* ���"uart �����ж�"δʹ�ܵĻ���ʹ��"uart �����ж�" */
	uart_tx_int_enable();
}


void do_uart_irq(void)
{
	int i;
	int cnt;
	unsigned char c;
	
	if (UINTP0 & (1<<2))
	{
		/* ���ڷ����ж� */
		 if (isEmpty())
		 {
		 	/* ��ֹ�ж� */
			uart_tx_int_disable();
		 }
		 else
		 {
		 	/* �ӻ��ͻ�������ȡ������, �ŵ�TX FIFO��ȥ */
			cnt = (UFSTAT0 >> 8) & 0x3f;
			cnt = 64 - cnt;
			for (i = 0; i < cnt; i++)
			{
				if (getData(&c) == 0)
				{
					UTXH0 = c;
				}
				else
				{
					break;
				}
			}
		 }
	}
	else if (UINTP0 & (1<<0))
	{
		/* ���ڽ����ж�, ��RX FIFO��ȡ������ */
	}

	/* ���ж� */
	UINTP0 = 0xf;
}



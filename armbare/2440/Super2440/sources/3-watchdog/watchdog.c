/*
 *	Descriptions: S3C2440 Board, watchdog driver program
 *
 *	Author:	Pete & Charles
 *
 *	Date: March 5rd, 2012
 *
*/

/* ����LED1��GPIO�Ĵ��� */
#define GPFCON                      (*(volatile unsigned int *)0x56000050)
#define GPFDAT                      (*(volatile unsigned int *)0x56000054)

/* ����LED2, LED3, LED4��GPIO�Ĵ��� */
#define GPGCON                      (*(volatile unsigned int *)0x56000060)
#define GPGDAT                      (*(volatile unsigned int *)0x56000064)


/* �ӳٺ궨�� */
#define DELAY(times)    do {    \
                            int i, j;   \
                            for(i = 0; i < times; ++i) \
                            for(j = 0; j < 0x10000; ++j); \
                        }while(0)   \


/* LED��ʼ�� */
void led_init(void)
{
    // GPF6 LED1  [13:12]   01 = Output
    GPFCON |= 1<<12;
    GPFCON &= ~(1<<13);
                            
    // GPG0 LED2    [1:0]       01 = Output
    GPGCON |= 1<<0;
    GPGCON &= ~(1<<1);
                                            
    // GPG1 LED3    [3:2]       01 = Output
    GPGCON |= 1<<2;
    GPGCON &= ~(1<<3);
                                                            
    // GPG10 LED4   [21:20]     01 = Output
    GPGCON |= 1<<20;
    GPGCON &= ~(1<<21);
                                                                            
    // ��ȫ��
    GPFDAT |= 1<<6;
    GPGDAT |= (1<<0 | 1<<1 | 1<<10);
}

#if 0
/* ��������һ��LED���� */
void led_on(int num)
{
    if(num == 1)
        GPFDAT &= ~(1<<(num+5));
    else if(num == 2)
        GPGDAT &= ~(1<<(num-2));
    else if(num == 3)
        GPGDAT &= ~(1<<(num-2));
    else if(num == 4)
        GPGDAT &= ~(1<<(num+6));
    else {
        // ��ȫ��
        GPFDAT |= 1<<6;
        GPGDAT |= (1<<0 | 1<<1 | 1<<10);
    }   
}


/* ��������һ��LED���� */
void led_off(int num)
{
    if(num == 1)
        GPFDAT |= 1<<(num+5);
    else if(num == 2)
        GPGDAT |= 1<<(num-2);
    else if(num == 3)
        GPGDAT |= 1<<(num-2);
    else if(num == 4)
        GPGDAT |= 1<<(num+6);
    else {
        // ��ȫ��
        GPFDAT |= 1<<6;
        GPGDAT |= (1<<0 | 1<<1 | 1<<10);
    }
}
#endif


/* LED��ȫ�� */
void led_all_off(void)
{
    GPFDAT |= 1<<6;
    GPGDAT |= (1<<0 | 1<<1 | 1<<10);
}


/* LED��ȫ�� */
void led_all_on(void)
{
    GPFDAT &= ~(1<<6);
    GPGDAT &= ~(1<<0 | 1<<1 | 1<<10);
}


/* ������ */
int mymain(void)
{
		
	led_init();

	led_all_on();
	DELAY(1);
	led_all_off();

	return 0;	
}


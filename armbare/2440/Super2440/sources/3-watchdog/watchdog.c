/*
 *	Descriptions: S3C2440 Board, watchdog driver program
 *
 *	Author:	Pete & Charles
 *
 *	Date: March 5rd, 2012
 *
*/

/* 控制LED1的GPIO寄存器 */
#define GPFCON                      (*(volatile unsigned int *)0x56000050)
#define GPFDAT                      (*(volatile unsigned int *)0x56000054)

/* 控制LED2, LED3, LED4的GPIO寄存器 */
#define GPGCON                      (*(volatile unsigned int *)0x56000060)
#define GPGDAT                      (*(volatile unsigned int *)0x56000064)


/* 延迟宏定义 */
#define DELAY(times)    do {    \
                            int i, j;   \
                            for(i = 0; i < times; ++i) \
                            for(j = 0; j < 0x10000; ++j); \
                        }while(0)   \


/* LED初始化 */
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
                                                                            
    // 灯全灭
    GPFDAT |= 1<<6;
    GPGDAT |= (1<<0 | 1<<1 | 1<<10);
}

#if 0
/* 控制其中一个LED灯亮 */
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
        // 灯全灭
        GPFDAT |= 1<<6;
        GPGDAT |= (1<<0 | 1<<1 | 1<<10);
    }   
}


/* 控制其中一个LED灯灭 */
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
        // 灯全灭
        GPFDAT |= 1<<6;
        GPGDAT |= (1<<0 | 1<<1 | 1<<10);
    }
}
#endif


/* LED灯全灭 */
void led_all_off(void)
{
    GPFDAT |= 1<<6;
    GPGDAT |= (1<<0 | 1<<1 | 1<<10);
}


/* LED灯全亮 */
void led_all_on(void)
{
    GPFDAT &= ~(1<<6);
    GPGDAT &= ~(1<<0 | 1<<1 | 1<<10);
}


/* 主函数 */
int mymain(void)
{
		
	led_init();

	led_all_on();
	DELAY(1);
	led_all_off();

	return 0;	
}


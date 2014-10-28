
#include <linux/module.h>  
#include <linux/kernel.h>
#include <linux/init.h> 
#include <linux/fs.h>   
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>             /* kmalloc/kfree */
#include <linux/miscdevice.h>
#include <linux/clk.h>              /* clk_get */
#include <asm/uaccess.h>
#include <asm/io.h>  
#include <asm/irq.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <mach/map.h>
#include <mach/regs-gpio.h>
#include <plat/regs-timer.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pete&Charles <http://embeddedfansclub.taobao.com>");


#define MINI2440_PWM_BUZZER_NAME     "mini2440_pwm_buzzer"

#define PWM_IOCTL_SET_FREQ		1
#define PWM_IOCTL_STOP			0

static struct semaphore lock;

/*  freq:  pclk/50/16/65536 ~ pclk/50/16 
 *  if pclk = 50MHz, freq is 1Hz to 62500Hz
 *  human ear : 20Hz~ 20000Hz
 */
static void PWM_Set_Freq( unsigned long freq )
{
	unsigned long tcon;
	unsigned long tcnt;
	unsigned long tcfg1;
	unsigned long tcfg0;

	struct clk *clk_p;
	unsigned long pclk;

	unsigned tmp;

    /* buzzer setup */
	tmp = readl(S3C2410_GPBCON);
	tmp &= ~(0x3U);
	tmp |=  (0x2U);
	writel(tmp, S3C2410_GPBCON);

	tcon  = __raw_readl(S3C2410_TCON);
	tcfg1 = __raw_readl(S3C2410_TCFG1);
	tcfg0 = __raw_readl(S3C2410_TCFG0);

	/* prescaler = 50 */
	tcfg0 &= ~S3C2410_TCFG_PRESCALER0_MASK;
	tcfg0 |= (50 - 1); 

	/* mux = 1/16 */
	tcfg1 &= ~S3C2410_TCFG1_MUX0_MASK;
	tcfg1 |= S3C2410_TCFG1_MUX0_DIV16;

	__raw_writel(tcfg1, S3C2410_TCFG1);
	__raw_writel(tcfg0, S3C2410_TCFG0);

	clk_p = clk_get(NULL, "pclk");
	pclk  = clk_get_rate(clk_p);
	tcnt  = (pclk/50/16)/freq;
	
	__raw_writel(tcnt, S3C2410_TCNTB(0));
	__raw_writel(tcnt/2, S3C2410_TCMPB(0));
				
	tcon &= ~0x1f;
	//tcon |= 0xb;		//disable deadzone, auto-reload, inv-off, update TCNTB0&TCMPB0, start timer 0
    tcon |= (S3C2410_TCON_T0START | S3C2410_TCON_T0MANUALUPD | S3C2410_TCON_T0RELOAD);
    __raw_writel(tcon, S3C2410_TCON);
	
	//tcon &= ~2;			//clear manual update bit
    tcon &= ~S3C2410_TCON_T0MANUALUPD;
    __raw_writel(tcon, S3C2410_TCON);
}


void PWM_Buzzer_Stop( void )
{
	unsigned tmp;
	tmp = readl(S3C2410_GPBCON);
	tmp &= ~(0x3U);
	writel(tmp, S3C2410_GPBCON);
}

static int mini2440_pwm_buzzer_open(struct inode *inode, struct file *filp)
{
	if (!down_trylock(&lock))
		return 0;
	else
		return -EBUSY;
}


static int mini2440_pwm_buzzer_close(struct inode *inode, struct file *filp)
{
	up(&lock);
	return 0;
}


static long mini2440_pwm_buzzer_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
		case PWM_IOCTL_SET_FREQ:
			if (arg == 0)
				return -EINVAL;
			PWM_Set_Freq(arg);
			break;

		case PWM_IOCTL_STOP:
		default:
			PWM_Buzzer_Stop();
			break;
	}

	return 0;
}


static struct file_operations mini2440_pwm_buzzer_fops = {
    .owner			= THIS_MODULE,
    .open			= mini2440_pwm_buzzer_open,
    .release		= mini2440_pwm_buzzer_close, 
    .unlocked_ioctl	= mini2440_pwm_buzzer_ioctl,
};

static struct miscdevice mini2440_pwm_buzzer_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = MINI2440_PWM_BUZZER_NAME,
	.fops = &mini2440_pwm_buzzer_fops,
};

static int __init mini2440_pwm_buzzer_init(void)
{
	int ret;

	sema_init(&lock, 1);
	ret = misc_register(&mini2440_pwm_buzzer_misc);

	printk (MINI2440_PWM_BUZZER_NAME" is initialized!!\n");
    	return ret;
}

static void __exit mini2440_pwm_buzzer_exit(void)
{
	misc_deregister(&mini2440_pwm_buzzer_misc);
}

module_init(mini2440_pwm_buzzer_init);
module_exit(mini2440_pwm_buzzer_exit);


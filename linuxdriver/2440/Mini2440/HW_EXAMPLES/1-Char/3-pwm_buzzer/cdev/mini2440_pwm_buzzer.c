
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


#define MINI2440_PWM_BUZZER_MAJOR               0
#define MINI2440_PWM_BUZZER_NAME                "mini2440_pwm_buzzer"

#define PWM_IOCTL_SET_FREQ		                1
#define PWM_IOCTL_STOP			                0

static int mini2440_pwm_buzzer_major = MINI2440_PWM_BUZZER_MAJOR;

struct mini2440_pwm_buzzer_dev {
    const char *pwm_buzzer_name;
    struct cdev cdev;
    struct class *pwm_buzzer_cls;
    struct device *pwm_buzzer_dev;
    struct semaphore pwm_buzzer_lock;
};

static struct mini2440_pwm_buzzer_dev *mini2440_pwm_buzzer_devp;

static void mini2440_pwm_buzzer_hw_init(void)
{    
    unsigned long tmp;
    
    /* buzzer setup */
    tmp = readl(S3C2410_GPBCON);
    tmp &= ~(0x3U);
    tmp |=  (0x2U);     /* TOUT0 */
    writel(tmp, S3C2410_GPBCON);
}


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
    
    /* PWM_Buzzer hardware related initialization */
    mini2440_pwm_buzzer_hw_init();

	tcon = __raw_readl(S3C2410_TCON);
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
    struct mini2440_pwm_buzzer_dev *dev = mini2440_pwm_buzzer_devp;
    filp->private_data = mini2440_pwm_buzzer_devp;
    
	if (!down_trylock(&dev->pwm_buzzer_lock))
		return 0;
	else
		return -EBUSY;
}

static int mini2440_pwm_buzzer_close(struct inode *inode, struct file *filp)
{
    struct mini2440_pwm_buzzer_dev *dev = filp->private_data;
    
	up(&dev->pwm_buzzer_lock);
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


static void mini2440_pwm_buzzer_setup_cdev(struct mini2440_pwm_buzzer_dev *dev, 
        int minor)
{
    int error;
    dev_t devno = MKDEV(mini2440_pwm_buzzer_major, minor);
    
    /* Initializing cdev */
    cdev_init(&dev->cdev, &mini2440_pwm_buzzer_fops);
    dev->cdev.owner = THIS_MODULE;

    /* Adding cdev */
    error = cdev_add(&dev->cdev, devno, 1);

    if (error) {
        printk(KERN_NOTICE "[KERNEL(mini2440_pwm_buzzer_setup_cdev)]Error %d adding pwm_buzzer", error);
    }
}

static int __init mini2440_pwm_buzzer_init(void)
{
	int ret = 0;
    dev_t devno = MKDEV(mini2440_pwm_buzzer_major, 0);

    /* Allocating mini2440_pwm_buzzer_dev structure dynamically */
    mini2440_pwm_buzzer_devp = kmalloc(sizeof(struct mini2440_pwm_buzzer_dev), GFP_KERNEL);
    if (!mini2440_pwm_buzzer_devp) {
        return -ENOMEM;
    }

    memset(mini2440_pwm_buzzer_devp, 0, sizeof(struct mini2440_pwm_buzzer_dev));

	sema_init(&mini2440_pwm_buzzer_devp->pwm_buzzer_lock, 1);

    mini2440_pwm_buzzer_devp->pwm_buzzer_name = MINI2440_PWM_BUZZER_NAME;

    /* Register char devices region */
    if (mini2440_pwm_buzzer_major) {
        ret = register_chrdev_region(devno, 1, mini2440_pwm_buzzer_devp->pwm_buzzer_name);
    }else {
        /* Allocating major number dynamically */
        ret = alloc_chrdev_region(&devno, 0, 1, mini2440_pwm_buzzer_devp->pwm_buzzer_name);
        mini2440_pwm_buzzer_major = MAJOR(devno);
    }

    if (ret < 0)
        return ret;

    /* Helper function to initialize and add cdev structure */
    mini2440_pwm_buzzer_setup_cdev(mini2440_pwm_buzzer_devp, 0);
    
    /* mdev - automatically create the device node */
    mini2440_pwm_buzzer_devp->pwm_buzzer_cls = class_create(THIS_MODULE, mini2440_pwm_buzzer_devp->pwm_buzzer_name);
    if (IS_ERR(mini2440_pwm_buzzer_devp->pwm_buzzer_cls))
        return PTR_ERR(mini2440_pwm_buzzer_devp->pwm_buzzer_cls);

    mini2440_pwm_buzzer_devp->pwm_buzzer_dev = device_create(mini2440_pwm_buzzer_devp->pwm_buzzer_cls, NULL, devno, NULL, mini2440_pwm_buzzer_devp->pwm_buzzer_name);    
	if (IS_ERR(mini2440_pwm_buzzer_devp->pwm_buzzer_dev)) {
        class_destroy(mini2440_pwm_buzzer_devp->pwm_buzzer_cls);
        cdev_del(&mini2440_pwm_buzzer_devp->cdev);
        unregister_chrdev_region(devno, 1);
        kfree(mini2440_pwm_buzzer_devp);
		return PTR_ERR(mini2440_pwm_buzzer_devp->pwm_buzzer_dev);
	}


	printk (MINI2440_PWM_BUZZER_NAME" is initialized!!\n");

    return ret;
}

static void __exit mini2440_pwm_buzzer_exit(void)
{    
    device_destroy(mini2440_pwm_buzzer_devp->pwm_buzzer_cls, MKDEV(mini2440_pwm_buzzer_major, 0));
    class_destroy(mini2440_pwm_buzzer_devp->pwm_buzzer_cls);
    cdev_del(&mini2440_pwm_buzzer_devp->cdev);
    unregister_chrdev_region(MKDEV(mini2440_pwm_buzzer_major, 0), 1);
    kfree(mini2440_pwm_buzzer_devp);    
}

module_init(mini2440_pwm_buzzer_init);
module_exit(mini2440_pwm_buzzer_exit);


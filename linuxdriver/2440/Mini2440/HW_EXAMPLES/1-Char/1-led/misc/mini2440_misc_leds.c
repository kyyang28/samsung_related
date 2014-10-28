

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
#include <asm/uaccess.h>
#include <asm/io.h>  
#include <asm/irq.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <mach/map.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pete&Charles <http://embeddedfansclub.taobao.com>");


#define MINI2440_LEDS_NAME          "mini2440_leds"

#define LED_1                       1
#define LED_2                       2
#define LED_3                       3
#define LED_4                       4
#define LED_ALL                     5


static long mini2440_led_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    unsigned long tmp;    

    printk("[DRIVER]Invoking mini2440_leds_ioctl!\n");

    switch(cmd) {        
    case LED_1:
    case LED_2:    
    case LED_3:    
    case LED_4:    
        tmp = readw(S3C2410_GPBDAT); 
        tmp = (tmp & ~(1<<(cmd + 4))) | (arg<<(cmd + 4));    
        writew(tmp, S3C2410_GPBDAT);
        break;

    case LED_ALL:
        tmp = readw(S3C2410_GPBDAT);
        if (arg)        /* LED_ALL_ON */
            tmp |= (0xF<<5);
        else            /* LED_ALL_OFF */
            tmp &= ~(0xF<<5);
        writew(tmp, S3C2410_GPBDAT);
        break;
        
    default:    
        return -EINVAL;    
    }
    
    return 0;        
}

static struct file_operations mini2440_misc_led_fops = {
	.owner			= THIS_MODULE,
	.unlocked_ioctl	= mini2440_led_ioctl,
};

static struct miscdevice mini2440_led_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = MINI2440_LEDS_NAME,
	.fops = &mini2440_misc_led_fops,
};

static void mini2440_leds_hw_init(void)
{    
	unsigned long tmp;

	tmp = readl(S3C2410_GPBCON);
    tmp = (tmp & ~(0xFFU << 10)) | (0x55U << 10);
	writel(tmp, S3C2410_GPBCON);
	
	tmp = readw(S3C2410_GPBDAT);
    tmp |= (0xF << 5);              /* lights all off */
	writew(tmp, S3C2410_GPBDAT);
}

static int __init mini2440_misc_led_init(void)
{
	int ret;

    /* Leds hardware related initialization */
    mini2440_leds_hw_init();

	ret = misc_register(&mini2440_led_misc);

	printk(MINI2440_LEDS_NAME" is initialized\n");

	return ret;
}

static void __exit mini2440_misc_led_exit(void)
{
	misc_deregister(&mini2440_led_misc);
}

module_init(mini2440_misc_led_init);
module_exit(mini2440_misc_led_exit);


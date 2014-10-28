

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
#include <mach/gpio-bank-k.h>
#include <mach/map.h>

#define TINY6410_LEDS_NAME          "tiny6410_leds"
#define LED_1                       1
#define LED_2                       2
#define LED_3                       3
#define LED_4                       4

static long tiny6410_led_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    unsigned long tmp;    

    printk("[DRIVER]Invoking tiny6410_leds_ioctl!\n");
    switch(cmd) {        
    case LED_1:    
    case LED_2:    
    case LED_3:    
    case LED_4:    
        tmp = readw(S3C64XX_GPKDAT);    
        tmp = (tmp & ~(1<<(cmd - 1 + 4))) | (arg<<(cmd - 1 + 4));    
        writew(tmp, S3C64XX_GPKDAT);    
        return 0;    
    default:    
        return -EINVAL;    
    }    
}

static struct file_operations tiny6410_misc_led_fops = {
	.owner			= THIS_MODULE,
	.unlocked_ioctl	= tiny6410_led_ioctl,
};

static struct miscdevice tiny6410_led_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = TINY6410_LEDS_NAME,
	.fops = &tiny6410_misc_led_fops,
};

static void tiny6410_leds_hw_init(void)
{    
	unsigned long tmp;

	tmp = readl(S3C64XX_GPKCON);
	tmp = (tmp & ~(0xffffU << 16)) | (0x1111U << 16);
	writel(tmp, S3C64XX_GPKCON);
	
	tmp = readw(S3C64XX_GPKDAT);
    tmp |= (0xF << 4);  /* lights all off */
	writew(tmp, S3C64XX_GPKDAT);
}

static int __init tiny6410_misc_led_init(void)
{
	int ret;

    /* Leds hardware related initialization */
    tiny6410_leds_hw_init();

	ret = misc_register(&tiny6410_led_misc);

	printk(TINY6410_LEDS_NAME" is initialized\n");

	return ret;
}

static void __exit tiny6410_misc_led_exit(void)
{
	misc_deregister(&tiny6410_led_misc);
}

module_init(tiny6410_misc_led_init);
module_exit(tiny6410_misc_led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pete&Charles <http://embeddedfansclub.taobao.com>");


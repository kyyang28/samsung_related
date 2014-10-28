

#include <linux/module.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/gpio_keys.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/errno.h>
#include <linux/ioport.h>

#include <asm/io.h> 
#include <asm/uaccess.h>


static int major;

static struct class *led_class;

static volatile unsigned long *GPIO_CON = NULL;
static volatile unsigned long *GPIO_DAT = NULL;
static int pin;

static int led_open(struct inode *inode, struct file *file)
{
    /* Configure GPF6, GPG0, GPG1, GPG10 */
	/* GPF6 LED1  [13:12]   01 = Output */
	*GPIO_CON &= ~((0x3<<(pin*2)));
	*GPIO_CON |= (0x1<<(pin*2));

    return 0;
}

static ssize_t led_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    int val;

	copy_from_user(&val, buf, count);

	if(val == 1) {
		/* light off */
        *GPIO_DAT |= (1<<pin);
        //*GPIO_DAT |= (1<<6);
		//*rGPGDAT |= (1<<0 | 1<<1 | 1<<10);
	}else {
		/* light on */
        *GPIO_DAT &= ~(1<<pin);
        //*GPIO_DAT &= ~(1<<6);
		//*rGPGDAT &= ~(1<<0 | 1<<1 | 1<<10);
	}
    
	return 0;
}


static struct file_operations led_fops = {
    .owner  = THIS_MODULE,
	.open   = led_open,
	.write  = led_write,
};

static int led_probe(struct platform_device *pdev)
{
    printk("led_drv.c: led_probe invoking!\n");    

    /* According to the resources of the platform_device, to do the ioremap() */
	struct resource *res;
    
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if(!res)
        return -ENODEV;

    GPIO_CON = ioremap(res->start, res->end - res->start + 1);
    GPIO_DAT = GPIO_CON + 1;

    res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
    pin = res->start;

    /* Register char device */
    major = register_chrdev(0, "myled", &led_fops);

    led_class = class_create(THIS_MODULE, "myled");
	if(IS_ERR(led_class))
		return PTR_ERR(led_class);
    
	if (IS_ERR(device_create(led_class, NULL, MKDEV(major, 0), NULL, "led")))
		printk("can't create device\n");

    return 0;
}


static int led_remove(struct platform_device *pdev)
{
    /* Unregister char device */   
    printk("led_drv.c: led_remove invoking!\n");

    device_destroy(led_class, MKDEV(major, 0));
    class_destroy(led_class);
    unregister_chrdev(major, "myled");

    /* According to the resources of the platform_device, to do the iounmap() */
    iounmap(GPIO_CON);

    return 0;
}


static struct platform_driver led_driver = {
	.probe		= led_probe,
	.remove		= led_remove,
	.driver		= {
		.name	= "myled",
		.owner	= THIS_MODULE,
	}
};


static int led_drv_init(void)
{
    platform_driver_register(&led_driver);   
    return 0;
}

static void led_drv_exit(void)
{
    platform_driver_unregister(&led_driver);
}

module_init(led_drv_init);
module_exit(led_drv_exit);

MODULE_LICENSE("GPL");


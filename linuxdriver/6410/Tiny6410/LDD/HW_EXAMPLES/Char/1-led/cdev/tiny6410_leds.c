
#include <linux/module.h>  
#include <linux/kernel.h>
#include <linux/init.h> 
#include <linux/fs.h>   
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>             /* kmalloc/kfree */
#include <asm/uaccess.h>
#include <asm/io.h>  
#include <asm/irq.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <mach/gpio-bank-k.h>
#include <mach/map.h>


#define TINY6410_LEDS_MAJOR         252
#define TINY6410_LEDS_NAME          "tiny6410_leds"

#define LED_1                       1
#define LED_2                       2
#define LED_3                       3
#define LED_4                       4

static int tiny6410_leds_major = TINY6410_LEDS_MAJOR;

struct tiny6410_leds_dev {
    const char *leds_name;
    struct cdev cdev;
    struct class *leds_cls;
    struct device *leds_dev;
};

static struct tiny6410_leds_dev *tiny6410_leds_devp;

static int tiny6410_leds_open(struct inode *inode, struct file *filp)
{    
    printk("[DRIVER]Invoking tiny6410_leds_open\n");
    filp->private_data = tiny6410_leds_devp;
    return 0;
}

static long tiny6410_leds_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)    
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


static struct file_operations tiny6410_leds_fops = {
    .owner              = THIS_MODULE,
    .open               = tiny6410_leds_open,
    .unlocked_ioctl     = tiny6410_leds_ioctl,
};

static void tiny6410_leds_setup_cdev(struct tiny6410_leds_dev *dev, 
        int minor)
{
    int error;
    dev_t devno = MKDEV(tiny6410_leds_major, minor);
    
    /* Initializing cdev */
    cdev_init(&dev->cdev, &tiny6410_leds_fops);
    dev->cdev.owner = THIS_MODULE;

    /* Adding cdev */
    error = cdev_add(&dev->cdev, devno, 1);

    if (error) {
        printk(KERN_NOTICE "[KERNEL(tiny6410_leds_setup_cdev)]Error %d adding leds", error);
    }
}

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

static int __init tiny6410_leds_init(void)
{
	int ret = 0;
    dev_t devno = MKDEV(tiny6410_leds_major, 0);

    /* Allocating tiny6410_leds_dev structure dynamically */
    tiny6410_leds_devp = kmalloc(sizeof(struct tiny6410_leds_dev), GFP_KERNEL);
    if (!tiny6410_leds_devp) {
        return -ENOMEM;
    }

    memset(tiny6410_leds_devp, 0, sizeof(struct tiny6410_leds_dev));

    tiny6410_leds_devp->leds_name = TINY6410_LEDS_NAME;

    /* Register char devices region */
    if (tiny6410_leds_major) {
        ret = register_chrdev_region(devno, 1, tiny6410_leds_devp->leds_name);
    }else {
        /* Allocating major number dynamically */
        ret = alloc_chrdev_region(&devno, 0, 1, tiny6410_leds_devp->leds_name);
        tiny6410_leds_major = MAJOR(devno);
    }

    if (ret < 0)
        return ret;

    
    /* Helper function to initialize and add cdev structure */
    tiny6410_leds_setup_cdev(tiny6410_leds_devp, 0);

    /* Leds hardware related initialization */
    tiny6410_leds_hw_init();

    /* mdev - automatically create the device node */
    tiny6410_leds_devp->leds_cls = class_create(THIS_MODULE, tiny6410_leds_devp->leds_name);
    if (IS_ERR(tiny6410_leds_devp->leds_cls))
        return PTR_ERR(tiny6410_leds_devp->leds_cls);

    tiny6410_leds_devp->leds_dev = device_create(tiny6410_leds_devp->leds_cls, NULL, devno, NULL, tiny6410_leds_devp->leds_name);    
	if (IS_ERR(tiny6410_leds_devp->leds_dev)) {
        class_destroy(tiny6410_leds_devp->leds_cls);
        cdev_del(&tiny6410_leds_devp->cdev);
        unregister_chrdev_region(devno, 1);
        kfree(tiny6410_leds_devp);
		return PTR_ERR(tiny6410_leds_devp->leds_dev);
	}

	printk(TINY6410_LEDS_NAME" is initialized!!\n");
    
    return ret;
}

static void __exit tiny6410_leds_exit(void)
{
    device_destroy(tiny6410_leds_devp->leds_cls, MKDEV(tiny6410_leds_major, 0));
    class_destroy(tiny6410_leds_devp->leds_cls);
    cdev_del(&tiny6410_leds_devp->cdev);
    unregister_chrdev_region(MKDEV(tiny6410_leds_major, 0), 1);
    kfree(tiny6410_leds_devp);    
}

module_init(tiny6410_leds_init);
module_exit(tiny6410_leds_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pete&Charles <http://embeddedfansclub.taobao.com>");


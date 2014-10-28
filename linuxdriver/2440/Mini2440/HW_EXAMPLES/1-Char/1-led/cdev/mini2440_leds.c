
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
#include <mach/map.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pete&Charles <http://embeddedfansclub.taobao.com>");


#define mini2440_LEDS_MAJOR         0
#define mini2440_LEDS_NAME          "mini2440_leds"

#define LED_1                       1
#define LED_2                       2
#define LED_3                       3
#define LED_4                       4
#define LED_ALL                     5

static int mini2440_leds_major = mini2440_LEDS_MAJOR;

struct mini2440_leds_dev {
    const char *leds_name;
    struct cdev cdev;
    struct class *leds_cls;
    struct device *leds_dev;
};

static struct mini2440_leds_dev *mini2440_leds_devp;

static int mini2440_leds_open(struct inode *inode, struct file *filp)
{    
    printk("[DRIVER]Invoking mini2440_leds_open\n");
    filp->private_data = mini2440_leds_devp;
    return 0;
}

static long mini2440_leds_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)    
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


static struct file_operations mini2440_leds_fops = {
    .owner              = THIS_MODULE,
    .open               = mini2440_leds_open,
    .unlocked_ioctl     = mini2440_leds_ioctl,
};

static void mini2440_leds_setup_cdev(struct mini2440_leds_dev *dev, 
        int minor)
{
    int error;
    dev_t devno = MKDEV(mini2440_leds_major, minor);
    
    /* Initializing cdev */
    cdev_init(&dev->cdev, &mini2440_leds_fops);
    dev->cdev.owner = THIS_MODULE;

    /* Adding cdev */
    error = cdev_add(&dev->cdev, devno, 1);

    if (error) {
        printk(KERN_NOTICE "[KERNEL(mini2440_leds_setup_cdev)]Error %d adding leds", error);
    }
}

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

static int __init mini2440_leds_init(void)
{
	int ret = 0;
    dev_t devno = MKDEV(mini2440_leds_major, 0);

    /* Allocating mini2440_leds_dev structure dynamically */
    mini2440_leds_devp = kmalloc(sizeof(struct mini2440_leds_dev), GFP_KERNEL);
    if (!mini2440_leds_devp) {
        return -ENOMEM;
    }

    memset(mini2440_leds_devp, 0, sizeof(struct mini2440_leds_dev));

    mini2440_leds_devp->leds_name = mini2440_LEDS_NAME;

    /* Register char devices region */
    if (mini2440_leds_major) {
        ret = register_chrdev_region(devno, 1, mini2440_leds_devp->leds_name);
    }else {
        /* Allocating major number dynamically */
        ret = alloc_chrdev_region(&devno, 0, 1, mini2440_leds_devp->leds_name);
        mini2440_leds_major = MAJOR(devno);
    }

    if (ret < 0)
        return ret;

    
    /* Helper function to initialize and add cdev structure */
    mini2440_leds_setup_cdev(mini2440_leds_devp, 0);

    /* Leds hardware related initialization */
    mini2440_leds_hw_init();

    /* mdev - automatically create the device node */
    mini2440_leds_devp->leds_cls = class_create(THIS_MODULE, mini2440_leds_devp->leds_name);
    if (IS_ERR(mini2440_leds_devp->leds_cls))
        return PTR_ERR(mini2440_leds_devp->leds_cls);

    mini2440_leds_devp->leds_dev = device_create(mini2440_leds_devp->leds_cls, NULL, devno, NULL, mini2440_leds_devp->leds_name);    
	if (IS_ERR(mini2440_leds_devp->leds_dev)) {
        class_destroy(mini2440_leds_devp->leds_cls);
        cdev_del(&mini2440_leds_devp->cdev);
        unregister_chrdev_region(devno, 1);
        kfree(mini2440_leds_devp);
		return PTR_ERR(mini2440_leds_devp->leds_dev);
	}

	printk(mini2440_LEDS_NAME" is initialized!!\n");
    
    return ret;
}

static void __exit mini2440_leds_exit(void)
{
    device_destroy(mini2440_leds_devp->leds_cls, MKDEV(mini2440_leds_major, 0));
    class_destroy(mini2440_leds_devp->leds_cls);
    cdev_del(&mini2440_leds_devp->cdev);
    unregister_chrdev_region(MKDEV(mini2440_leds_major, 0), 1);
    kfree(mini2440_leds_devp);    
}

module_init(mini2440_leds_init);
module_exit(mini2440_leds_exit);


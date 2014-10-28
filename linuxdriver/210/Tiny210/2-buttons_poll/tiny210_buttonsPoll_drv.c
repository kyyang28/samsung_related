

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>       /* class_create / class_destroy */
#include <asm/io.h>             /* ioremap / iounmap */
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");

static struct cdev *tiny210_buttonsPoll_cdev;
static struct cdev *tiny210_leds_cdev;
static struct class *tiny210_buttonsPoll_cls;
static struct class *tiny210_leds_cls;
static dev_t tiny210_buttonsPoll_dev;
static dev_t tiny210_leds_dev;

/* the GPIO of the buttons */
static volatile unsigned long *GPH2CON = NULL;
static volatile unsigned long *GPH2DAT = NULL;
static volatile unsigned long *GPH3CON = NULL;
static volatile unsigned long *GPH3DAT = NULL;

/* the GPIO of the leds */
static volatile unsigned long *GPJ2CON = NULL;
static volatile unsigned long *GPJ2DAT = NULL;

static int tiny210_buttonsPoll_open(struct inode *inode, struct file *filp)
{
    *GPH2CON = 0x0<<12 | 0x0<<8 | 0x0<<4 | 0x0<<0;   /* GPH2CON = 0x0 */
    *GPH3CON = 0x0<<12 | 0x0<<8 | 0x0<<4 | 0x0<<0;   /* GPH3CON = 0x0 */
    return 0;
}

static ssize_t tiny210_buttonsPoll_read(struct file *filp, char __user *buf, 
            size_t count, loff_t *ppos)
{
    int kbuf[8];

    kbuf[0] = (*GPH2DAT & (1<<0)) ? 1 : 0;
    kbuf[1] = (*GPH2DAT & (1<<1)) ? 1 : 0;
    kbuf[2] = (*GPH2DAT & (1<<2)) ? 1 : 0;
    kbuf[3] = (*GPH2DAT & (1<<3)) ? 1 : 0;
    kbuf[4] = (*GPH3DAT & (1<<0)) ? 1 : 0;
    kbuf[5] = (*GPH3DAT & (1<<1)) ? 1 : 0;
    kbuf[6] = (*GPH3DAT & (1<<2)) ? 1 : 0;
    kbuf[7] = (*GPH3DAT & (1<<3)) ? 1 : 0;

    if (copy_to_user(buf, kbuf, count))
        return -EFAULT;
    
    return count;
}

static struct file_operations tiny210_buttonsPoll_fops = {
    .owner              = THIS_MODULE,
    .open               = tiny210_buttonsPoll_open,
    .read               = tiny210_buttonsPoll_read,
};

static int tiny210_leds_open(struct inode *inode, struct file *filp)
{
    *GPJ2CON = 0x1<<12 | 0x1<<8 | 0x1<<4 | 0x1<<0;
    return 0;
}

static long tiny210_leds_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    switch (arg) {
    case 1:
        if (!cmd)
            *GPJ2DAT &= ~(1<<0);
        else
            *GPJ2DAT |= (1<<0);
        break;

    case 2:
        if (!cmd)
            *GPJ2DAT &= ~(1<<1);
        else
            *GPJ2DAT |= (1<<1);
        break;

    case 3:
        if (!cmd)
            *GPJ2DAT &= ~(1<<2);
        else
            *GPJ2DAT |= (1<<2);
        break;

    case 4:
        if (!cmd)
            *GPJ2DAT &= ~(1<<3);
        else
            *GPJ2DAT |= (1<<3);
        break;

    default:
        return -EINVAL;
    }
    
    return 0;
}

static struct file_operations tiny210_leds_fops = {
    .owner              = THIS_MODULE,
    .open               = tiny210_leds_open,
    .unlocked_ioctl     = tiny210_leds_ioctl,
};

static int __init tiny210_buttonsPoll_drv_init(void)
{
    int retval;

    retval = alloc_chrdev_region(&tiny210_buttonsPoll_dev, 0, 1, "tiny210ButtonsPoll");
	if (retval < 0) {
		printk(KERN_WARNING "tiny210_buttonsPoll_drv_init: can't get buttonsPoll major number\n");
		return retval;
	}

    retval = alloc_chrdev_region(&tiny210_leds_dev, 0, 1, "tiny210Leds");
	if (retval < 0) {
		printk(KERN_WARNING "tiny210_buttonsPoll_drv_init: can't get leds major number\n");
		return retval;
	}
    
    /* Seq 1: Allocating the cdev struct */
    tiny210_buttonsPoll_cdev = cdev_alloc();
    tiny210_leds_cdev = cdev_alloc();

    /* Seq 2: Initializing the cdev struct */
    cdev_init(tiny210_buttonsPoll_cdev, &tiny210_buttonsPoll_fops);
    tiny210_buttonsPoll_cdev->owner = tiny210_buttonsPoll_fops.owner;

    cdev_init(tiny210_leds_cdev, &tiny210_leds_fops);
    tiny210_leds_cdev->owner = tiny210_leds_fops.owner;

    /* Seq 3: Adding the cdev struct */
    cdev_add(tiny210_buttonsPoll_cdev, tiny210_buttonsPoll_dev, 1);
    cdev_add(tiny210_leds_cdev, tiny210_leds_dev, 1);

    /* Seq 4: Class create */
    tiny210_buttonsPoll_cls = class_create(THIS_MODULE, "tiny210ButtonsPoll");
    device_create(tiny210_buttonsPoll_cls, NULL, tiny210_buttonsPoll_dev, NULL, "tiny210_buttonsPoll");

    tiny210_leds_cls = class_create(THIS_MODULE, "tiny210Leds");
    device_create(tiny210_leds_cls, NULL, tiny210_leds_dev, NULL, "tiny210_leds");

    /* Seq 5: Hardware related setup */
    GPH2CON = (volatile unsigned long *)ioremap(0xE0200C40, 8);
    GPH2DAT = GPH2CON + 1;

    GPH3CON = (volatile unsigned long *)ioremap(0xE0200C60, 8);
    GPH3DAT = GPH3CON + 1;

    GPJ2CON = (volatile unsigned long *)ioremap(0xE0200280, 8);
    GPJ2DAT = GPJ2CON + 1;

    printk("tiny210_buttonsPoll_leds module is inserted into the kernel\n");
    
    return 0;
}
module_init(tiny210_buttonsPoll_drv_init);

static void __exit tiny210_buttonsPoll_drv_exit(void)
{
    iounmap(GPJ2CON);
    iounmap(GPH3CON);
    iounmap(GPH2CON);
    device_destroy(tiny210_leds_cls, tiny210_leds_dev);
    class_destroy(tiny210_leds_cls);
    device_destroy(tiny210_buttonsPoll_cls, tiny210_buttonsPoll_dev);
    class_destroy(tiny210_buttonsPoll_cls);
    cdev_del(tiny210_leds_cdev);
    cdev_del(tiny210_buttonsPoll_cdev);
    unregister_chrdev_region(tiny210_leds_dev, 1);
    unregister_chrdev_region(tiny210_buttonsPoll_dev, 1);
    
    printk("tiny210_buttonsPoll_leds module is removed from the kernel\n");
}
module_exit(tiny210_buttonsPoll_drv_exit);


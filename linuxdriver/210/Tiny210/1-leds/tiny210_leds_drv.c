
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/io.h>

MODULE_LICENSE("GPL");

static int tiny210_leds_major;
static struct cdev *cdev;
static struct class *tiny210_leds_cls;

static volatile unsigned long *GPJ2CON = NULL;
static volatile unsigned long *GPJ2DAT = NULL;

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

static int __init tiny210_leds_init(void)
{
    int ret;
    dev_t dev;

    ret = alloc_chrdev_region(&dev, 0, 1, "tiny210_leds");
    if (ret < 0)
        return ret;

    tiny210_leds_major = MAJOR(dev);

    /* Seq 1: Allocating the cdev struct */    
    cdev = cdev_alloc();

    /* Seq 2: Initializing the cdev struct */
    cdev_init(cdev, &tiny210_leds_fops);
    cdev->owner = tiny210_leds_fops.owner;

    /* Seq 3: Adding the cdev struct */
    cdev_add(cdev, dev, 1);

    /* mdev */
    tiny210_leds_cls = class_create(THIS_MODULE, "tiny210Leds");
    device_create(tiny210_leds_cls, NULL, dev, NULL, "tiny210_leds");   /* /dev/tiny210_leds */

    GPJ2CON = (volatile unsigned long *)ioremap(0xE0200280, 8);
    GPJ2DAT = GPJ2CON + 1;

    printk("tiny210_leds module is inserted into the kernel\n");
    
    return 0;
}
module_init(tiny210_leds_init);

static void __exit tiny210_leds_exit(void)
{
    iounmap(GPJ2CON);
    device_destroy(tiny210_leds_cls, MKDEV(tiny210_leds_major, 0));
    class_destroy(tiny210_leds_cls);
    cdev_del(cdev);
    unregister_chrdev_region(MKDEV(tiny210_leds_major, 0), 1);

    printk("tiny210_leds module is removed from the kernel\n");
}
module_exit(tiny210_leds_exit);


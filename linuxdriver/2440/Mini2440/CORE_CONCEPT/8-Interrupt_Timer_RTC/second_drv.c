
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>     /* kmalloc/kfree */
#include <linux/device.h>   /* class_create/class_destroy */
#include <asm/io.h>
#include <asm/system.h>
#include <asm/uaccess.h>


#define SECOND_MAJOR    252

static int second_major = SECOND_MAJOR;

struct second_dev {
    struct cdev cdev;   /* cdev structure */
    atomic_t counter;
    struct timer_list s_timer;
};

static struct second_dev *second_devp;
static struct class *second_cls;

static void second_timer_handler(unsigned long data)
{
    mod_timer(&second_devp->s_timer, jiffies + HZ); /* HZ = 1s */
    atomic_inc(&second_devp->counter);

    printk(KERN_NOTICE "current jiffies is %ld\n", jiffies);
}

static int second_open(struct inode *inode, struct file *filp)
{
    /* Initialize timer */
    init_timer(&second_devp->s_timer);
    second_devp->s_timer.function = second_timer_handler;
    second_devp->s_timer.expires  = jiffies + HZ;   /* HZ = 1s */

    /* Add(Register) timer */
    add_timer(&second_devp->s_timer);

    /* Clear the counter to zero */
    atomic_set(&second_devp->counter, 0);

    return 0;
}

static ssize_t second_read(struct file *filp, char __user *buf, size_t count, 
    loff_t *ppos)
{
    int cnt;

    cnt = atomic_read(&second_devp->counter);
    if (put_user(cnt, (int *)buf))
        return -EFAULT;
    else
        return sizeof(unsigned int);
}

static int second_release(struct inode *inode, struct file *filp)
{
    del_timer(&second_devp->s_timer);
    return 0;
}

static struct file_operations second_fops = {
    .owner      = THIS_MODULE,
    .open       = second_open,
    .read       = second_read,
    .release    = second_release,
};

static void second_setup_cdev(struct second_dev *dev, int index)
{
    int err, devno = MKDEV(second_major, index);
    cdev_init(&dev->cdev, &second_fops);
    dev->cdev.owner = THIS_MODULE;
    err = cdev_add(&dev->cdev, devno, 1);
    if (err) 
        printk(KERN_NOTICE "Error %d adding second cdev!\n", err);
}

static int __init second_init(void)
{
    int ret;
    dev_t devno = MKDEV(second_major, 0);

    /* ×¢²á×Ö·ûÉè±¸Çý¶¯ */
    if (second_major)
        ret = register_chrdev_region(devno, 1, "second");
    else {
        ret = alloc_chrdev_region(&devno, 0, 1, "second");
        second_major = MAJOR(devno);
    }

    if (ret < 0)
        return ret;

    second_devp = kmalloc(sizeof(struct second_dev), GFP_KERNEL);
    if (!second_devp) {
        ret = -ENOMEM;
        goto fail_malloc;
    }

    memset(second_devp, 0, sizeof(struct second_dev));
    
    second_setup_cdev(second_devp, 0);
   
    /* mdev - automatically create the device node */
    second_cls = class_create(THIS_MODULE, "second");
    if (IS_ERR(second_cls))
        return PTR_ERR(second_cls);

    device_create(second_cls, NULL, devno, NULL, "second"); /* /dev/second */
    
    return 0;
    
fail_malloc:
    unregister_chrdev_region(devno, 1);
    return ret;
}

static void __exit second_exit(void)
{
    device_destroy(second_cls, MKDEV(second_major, 0));
    class_destroy(second_cls);
    cdev_del(&second_devp->cdev);
    kfree(second_devp);
    unregister_chrdev_region(MKDEV(second_major, 0), 1);
}

module_init(second_init);
module_exit(second_exit);

MODULE_LICENSE("GPL");


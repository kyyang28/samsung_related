
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/device.h>   /* class_create */
#include <linux/slab.h>     /* kmalloc/kfree */
#include <asm/uaccess.h>


#define GLOBALMEM_SIZE  0x1000  /* global memory size 4KB */
#define GLOBALMEM_MAJOR 250     /* The major number */
#define MEM_CLEAR       0x1     /* The code to clear the memory region to zero */


static int globalmem_major = GLOBALMEM_MAJOR;

/* Our own globalmem_dev structure */
struct globalmem_dev {
    struct cdev cdev;
    unsigned char mem[GLOBALMEM_SIZE];
};

/* Object of globalmem_dev structure */
static struct globalmem_dev *globalmem_devp;

/* Class */
static struct class *globalmem_cls;

static int globalmem_open(struct inode *inode, struct file *filp)
{
    filp->private_data = globalmem_devp;
    return 0;
}

static int globalmem_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static loff_t globalmem_llseek(struct file *filp, loff_t offset, int orig)
{
    loff_t ret;

    switch (orig) {
    /* Offset from the beginning position of the file */
    case 0:
        if (offset < 0) {
            ret = -EINVAL;
            break;
        }
    
        if (offset > GLOBALMEM_SIZE) {
            ret = -EINVAL;
            break;
        }
    
        filp->f_pos = offset;
        ret = filp->f_pos;
        break;
        
    /* Offset from the current position of the file */
    case 1:
        if ((filp->f_pos + offset) < 0) {
            ret = -EINVAL;
            break;
        }

        if ((filp->f_pos + offset) > GLOBALMEM_SIZE) {
            ret = -EINVAL;
            break;
        }

        filp->f_pos += offset;
        ret = filp->f_pos;
        break;
        
    default:
        ret = -EINVAL;
    }
    
    return ret;
}

static ssize_t globalmem_read(struct file *filp, char __user *buf, size_t count, 
       loff_t *ppos)
{
    int ret = 0;
    unsigned long p = *ppos;
    struct globalmem_dev *dev = filp->private_data;
    
    /* The position which will be read is out of bound */
    if (p >= GLOBALMEM_SIZE)
        return 0;
    
    /* The byte which will be read is too big */
    if (count > GLOBALMEM_SIZE - p)
        count = GLOBALMEM_SIZE - p;
    
    /* Copy mem array from KERNEL space to USER space */
    if (copy_to_user(buf, (void *)(dev->mem + p), count))
        ret = -EFAULT;
    else {
        *ppos += count;
        ret = count;
    
        printk(KERN_INFO "[KERNEL(globalmem_read)]read %u bytes(s) from %lu\n", count, p);
    }
    
    return ret;
}

static ssize_t globalmem_write(struct file *filp, const char __user *buf, 
    size_t count, loff_t *ppos)
{
    int ret = 0;
    unsigned long p = *ppos;
    struct globalmem_dev *dev = filp->private_data;
    
    if (p >= GLOBALMEM_SIZE)
        return 0;
    
    if (count > GLOBALMEM_SIZE - p)
        count = GLOBALMEM_SIZE - p;
    
    if (copy_from_user(dev->mem + p, buf, count))
        ret = -EFAULT;
    else {
        *ppos += count;
        ret = count;
    
        printk(KERN_INFO "[KERNEL(globalmem_write)]written %u bytes(s) from %lu\n", count, p);
    }
    
    return ret;
}

static long globalmem_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct globalmem_dev *dev = filp->private_data;
    
    switch (cmd) {
    /* Clear the memory region to zero */
    case MEM_CLEAR:
        memset(dev->mem, 0, GLOBALMEM_SIZE);
        printk(KERN_INFO "[KERNEL(globalmem_ioctl)]globalmem is set to zero\n");
        break;

    default:
        return -EINVAL;
    }
    
    return 0;
}


static const struct file_operations globalmem_fops = {
    .owner              = THIS_MODULE,
    .llseek             = globalmem_llseek,
    .read               = globalmem_read,
    .write              = globalmem_write,
    .unlocked_ioctl     = globalmem_ioctl,
    .open               = globalmem_open,
    .release            = globalmem_release,
};

static void globalmem_setup_cdev(struct globalmem_dev *dev, int index)
{
    int err; 
    dev_t devno = MKDEV(globalmem_major, index);

    /* Initializing cdev */
    cdev_init(&dev->cdev, &globalmem_fops);
    dev->cdev.owner = THIS_MODULE;

    /* Adding cdev */
    err = cdev_add(&dev->cdev, devno, 1);

    if (err) {
        printk(KERN_NOTICE "[KERNEL(globalmem_setup_cdev)]Error %d adding globalmem", err);
    }
}

static int __init globalmem_init(void)
{
    int result;
    dev_t devno = MKDEV(globalmem_major, 0);
    
    /* Register char devices region */
    if (globalmem_major) {
        result = register_chrdev_region(devno, 1, "globalmem");
    }else {
        /* Allocating major number dynamically */
        result = alloc_chrdev_region(&devno, 0, 1, "globalmem");
        globalmem_major = MAJOR(devno);
    }

    if (result < 0)
        return result;

    /* Allocating globalmem_dev structure dynamically */
    globalmem_devp = kmalloc(sizeof(struct globalmem_dev), GFP_KERNEL);
    if (!globalmem_devp) {
        result = -ENOMEM;
        goto fail_malloc;
    }
    
    memset(globalmem_devp, 0, sizeof(struct globalmem_dev));
    
    /* Helper function to initialize and add cdev structure */
    globalmem_setup_cdev(globalmem_devp, 0);

    /* mdev - automatically create the device node */
    globalmem_cls = class_create(THIS_MODULE, "globalmem");
    if (IS_ERR(globalmem_cls))
        return PTR_ERR(globalmem_cls);

    device_create(globalmem_cls, NULL, devno, NULL, "globalmem");
        
    return 0;

fail_malloc:
    unregister_chrdev_region(devno, 1);
    return result;
}

static void __exit globalmem_exit(void)
{
    device_destroy(globalmem_cls, MKDEV(globalmem_major, 0));
    class_destroy(globalmem_cls);
    cdev_del(&globalmem_devp->cdev);
    kfree(globalmem_devp);
    unregister_chrdev_region(MKDEV(globalmem_major, 0), 1);
}

module_init(globalmem_init);
module_exit(globalmem_exit);

MODULE_LICENSE("GPL");


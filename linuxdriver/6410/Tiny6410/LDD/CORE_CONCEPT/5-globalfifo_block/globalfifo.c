

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/sched.h>        /* __set_current_state */
#include <linux/wait.h>         /* wait queue */
#include <linux/device.h>       /* class_create/class_destroy */
#include <linux/semaphore.h>    /* struct semaphore */
#include <linux/slab.h>         /* kmalloc/kfree */
#include <asm/uaccess.h>        /* copy_from_user/copy_to_user */


#define GLOBALFIFO_SIZE  0x1000  /* global fifo size 4KB */
#define GLOBALFIFO_MAJOR 250     /* The major number */
#define MEM_CLEAR       0x1     /* The code to clear the memory region to zero */


static int globalfifo_major = GLOBALFIFO_MAJOR;

/* Our own globalfifo_dev structure */
struct globalfifo_dev {
    struct cdev cdev;   /* cdev structure */
    unsigned int current_len;   /* valid fifo data length */
    unsigned char mem[GLOBALFIFO_SIZE]; /* global memory */
    struct semaphore sem;   /* semaphore structure */
    wait_queue_head_t r_wait;   /* wait queue head for read buffer */
    wait_queue_head_t w_wait;   /* wait queue head for write buffer */
};

/* Object of globalmem_dev structure */
static struct globalfifo_dev *globalfifo_devp;

/* Globalfifo class  */
static struct class *globalfifo_cls;

static int globalfifo_open(struct inode *inode, struct file *filp)
{
    filp->private_data = globalfifo_devp;
    return 0;
}

static int globalfifo_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static loff_t globalfifo_llseek(struct file *filp, loff_t offset, int orig)
{
    loff_t ret;

    switch (orig) {
    /* Offset from the beginning position of the file */
    case 0:
        if (offset < 0) {
            ret = -EINVAL;
            break;
        }
    
        if (offset > GLOBALFIFO_SIZE) {
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

        if ((filp->f_pos + offset) > GLOBALFIFO_SIZE) {
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

static ssize_t globalfifo_read(struct file *filp, char __user *buf, size_t count, 
       loff_t *ppos)
{
    int ret = 0;
    struct globalfifo_dev *dev = filp->private_data;

    /* Declare the wait queue */
    DECLARE_WAITQUEUE(wait, current);
      
    /* Acquire the semaphore */
    down(&dev->sem);

    /* Adding the r_wait to the wait queue */
    add_wait_queue(&dev->r_wait, &wait);

    /* Waiting the FIFO is not empty */
    while (dev->current_len == 0) {
        if (filp->f_flags & O_NONBLOCK) {
            ret = -EAGAIN;
            goto out;
        }

        /* Change the current process status to TASK_INTERRUPTIBLE */
        __set_current_state(TASK_INTERRUPTIBLE);
        up(&dev->sem);

        /* Schedule other process to run */
        schedule();
        
        if (signal_pending(current)) {
            ret = -ERESTARTSYS;
            goto out2;
        }

        /* Acquire the semaphore */
        down(&dev->sem);
    }

    if (count > dev->current_len)
        count = dev->current_len;
    
    /* Copy memory fifo from KERNEL space to USER space */
    if (copy_to_user(buf, dev->mem, count)) {
        ret = -EFAULT;
        goto out;
    }else {
        memcpy(dev->mem, dev->mem + count, dev->current_len - count);
        dev->current_len -= count;
        printk(KERN_INFO "[KERNEL(globalfifo_read)]read %u bytes(s), current_len:%u\n", 
            count, dev->current_len);
        
        /* Wakeup the write wait queue */
        wake_up_interruptible(&dev->w_wait);
        ret = count;
    }

out:
    /* Free the semaphore */
    up(&dev->sem);
out2:
    remove_wait_queue(&dev->r_wait, &wait);
    set_current_state(TASK_RUNNING);
    
    return ret;
}

static ssize_t globalfifo_write(struct file *filp, const char __user *buf, 
    size_t count, loff_t *ppos)
{
    int ret = 0;
    struct globalfifo_dev *dev = filp->private_data;

    /* Declare the wait queue */
    DECLARE_WAITQUEUE(wait, current);

    /* Acquire the semaphore */
    down(&dev->sem);

    /* Adding the write wait queue to the wait queue */
    add_wait_queue(&dev->w_wait, &wait);

    /* Waiting FIFO is not full */
    while (dev->current_len == GLOBALFIFO_SIZE) {
        if (filp->f_flags & O_NONBLOCK) {
            ret = -EAGAIN;
            goto out;
        }

        /* Set process state to TASK_INTERRUPTIBLE */
        __set_current_state(TASK_INTERRUPTIBLE);

        /* Free semaphore */
        up(&dev->sem);

        /* Schedule other process to run */
        schedule();

        if (signal_pending(current)) {
            ret = -ERESTARTSYS;
            goto out2;
        }

        /* Acquire the semaphore */
        down(&dev->sem);
    }

    if (count > GLOBALFIFO_SIZE - dev->current_len)
        count = GLOBALFIFO_SIZE - dev->current_len;
    
    if (copy_from_user(dev->mem + dev->current_len, buf, count)) {
        ret = -EFAULT;
        goto out;
    }else {
        dev->current_len += count;
        printk(KERN_INFO "[KERNEL(globalfifo_write)]written %u bytes(s), current_len:%u\n", 
            count, dev->current_len);

        /* Wakeup read wait queue */
        wake_up_interruptible(&dev->r_wait);

        ret = count;
    }

out:
    /* Free the semaphore */
    up(&dev->sem);
out2:
    remove_wait_queue(&dev->w_wait, &wait);
    set_current_state(TASK_RUNNING);
    
    return ret;
}

static long globalfifo_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct globalfifo_dev *dev = filp->private_data;
    
    switch (cmd) {
    /* Clear the memory region to zero */
    case MEM_CLEAR:
        /* Acquire the semaphore */
        if (down_interruptible(&dev->sem))
            return -ERESTARTSYS;
        
        memset(dev->mem, 0, GLOBALFIFO_SIZE);

        /* Free the semaphore */
        up(&dev->sem);

        printk(KERN_INFO "[KERNEL(globalfifo_ioctl)]globalfifo is set to zero\n");
        break;

    default:
        return -EINVAL;
    }
    
    return 0;
}


static const struct file_operations globalfifo_fops = {
    .owner              = THIS_MODULE,
    .llseek             = globalfifo_llseek,
    .read               = globalfifo_read,
    .write              = globalfifo_write,
    .unlocked_ioctl     = globalfifo_ioctl,
    .open               = globalfifo_open,
    .release            = globalfifo_release,
};

static void globalfifo_setup_cdev(struct globalfifo_dev *dev, int index)
{
    int err; 
    dev_t devno = MKDEV(globalfifo_major, index);

    /* Initializing cdev */
    cdev_init(&dev->cdev, &globalfifo_fops);
    dev->cdev.owner = THIS_MODULE;

    /* Adding cdev */
    err = cdev_add(&dev->cdev, devno, 1);

    if (err) {
        printk(KERN_NOTICE "[KERNEL(globalfifo_setup_cdev)]Error %d adding globalfifo", err);
    }
}

static int __init globalfifo_init(void)
{
    int result;
    dev_t devno = MKDEV(globalfifo_major, 0);
    
    /* Register char devices region */
    if (globalfifo_major) {
        result = register_chrdev_region(devno, 1, "globalfifo");
    }else {
        /* Allocating major number dynamically */
        result = alloc_chrdev_region(&devno, 0, 1, "globalfifo");
        globalfifo_major = MAJOR(devno);
    }

    if (result < 0)
        return result;

    /* Allocating globalmem_dev structure dynamically */
    globalfifo_devp = kmalloc(sizeof(struct globalfifo_dev), GFP_KERNEL);
    if (!globalfifo_devp) {
        result = -ENOMEM;
        goto fail_malloc;
    }
    
    memset(globalfifo_devp, 0, sizeof(struct globalfifo_dev));
    
    /* Helper function to initialize and add cdev structure */
    globalfifo_setup_cdev(globalfifo_devp, 0);

    /* mdev - automatically create the device node */
    globalfifo_cls = class_create(THIS_MODULE, "globalfifo");
    if (IS_ERR(globalfifo_cls))
        return PTR_ERR(globalfifo_cls);

    device_create(globalfifo_cls, NULL, devno, NULL, "globalfifo");

    /* Initialize semaphore structure */
    sema_init(&globalfifo_devp->sem, 1);

    /* Initialize wait_queue for read_wait_queue and write_wait_queue */
    init_waitqueue_head(&globalfifo_devp->r_wait);
    init_waitqueue_head(&globalfifo_devp->w_wait);
    
    return 0;

fail_malloc:
    unregister_chrdev_region(devno, 1);
    return result;
}

static void __exit globalfifo_exit(void)
{
    device_destroy(globalfifo_cls, MKDEV(globalfifo_major, 0));
    class_destroy(globalfifo_cls);
    cdev_del(&globalfifo_devp->cdev);
    kfree(globalfifo_devp);
    unregister_chrdev_region(MKDEV(globalfifo_major, 0), 1);
}

module_init(globalfifo_init);
module_exit(globalfifo_exit);

MODULE_LICENSE("GPL");


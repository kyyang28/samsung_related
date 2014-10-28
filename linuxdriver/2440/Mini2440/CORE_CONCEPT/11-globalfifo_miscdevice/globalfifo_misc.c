
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/poll.h>             /* poll_table/poll_wait */
#include <linux/sched.h>            /* __set_current_state */
#include <linux/wait.h>             /* wait queue */
#include <linux/device.h>           /* class_create/class_destroy */
#include <linux/semaphore.h>        /* struct semaphore */
#include <linux/slab.h>             /* kmalloc/kfree */
#include <linux/miscdevice.h>       /* miscdevice structure */
#include <asm/uaccess.h>            /* copy_from_user/copy_to_user */


#define GLOBALFIFO_SIZE     0x1000     /* global fifo size 4KB */
#define GLOBALFIFO_MINOR    128	    /* Major 10, Minor 128, /dev/globalfifo_misc */
#define FIFO_CLEAR          0x1        /* The code to clear the FIFO region to zero */


/* Our own globalfifo_dev structure */
struct globalfifo_dev {
    unsigned int current_len;               /* valid fifo data length */
    unsigned char mem[GLOBALFIFO_SIZE];     /* global memory */
    struct semaphore sem;                   /* semaphore structure */
    wait_queue_head_t r_wait;               /* wait queue head for read buffer */
    wait_queue_head_t w_wait;               /* wait queue head for write buffer */
    struct fasync_struct *async_queue;      /* async structure */
};

/* Object of globalmem_dev structure */
static struct globalfifo_dev *globalfifo_devp;

static int globalfifo_fasync(int fd, struct file *filp, int mode)
{
    struct globalfifo_dev *dev = filp->private_data;
    return fasync_helper(fd, filp, mode, &dev->async_queue);
}

static int globalfifo_open(struct inode *inode, struct file *filp)
{
    filp->private_data = globalfifo_devp;
    return 0;
}

static int globalfifo_release(struct inode *inode, struct file *filp)
{
    globalfifo_fasync(-1, filp, 0);
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

        if (dev->async_queue)
            kill_fasync(&dev->async_queue, SIGIO, POLL_IN);

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
    case FIFO_CLEAR:
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

static unsigned int globalfifo_poll(struct file *filp, 
    struct poll_table_struct *wait)
{
    unsigned int mask = 0;
    struct globalfifo_dev *dev = filp->private_data;

    /* Acquire the semaphore */
    down(&dev->sem);

    /* Adding the read/write wait queues to the wait queue */
    poll_wait(filp, &dev->r_wait, wait);
    poll_wait(filp, &dev->w_wait, wait);

    /* FIFO is not empty */
    if (dev->current_len != 0)
        /* Be able to read the data to the FIFO */
        mask |= POLLIN | POLLRDNORM;

    /* FIFO is not full */
    if (dev->current_len != GLOBALFIFO_SIZE)
        /* Be able to write the data to the FIFO */
        mask |= POLLOUT | POLLWRNORM;

    /* Free the semaphore */
    up(&dev->sem);

    return mask;
}


static const struct file_operations globalfifo_fops = {
    .owner              = THIS_MODULE,
    .llseek             = globalfifo_llseek,
    .read               = globalfifo_read,
    .write              = globalfifo_write,
    .unlocked_ioctl     = globalfifo_ioctl,
    .open               = globalfifo_open,
    .poll               = globalfifo_poll,
    .fasync             = globalfifo_fasync,
    .release            = globalfifo_release,
};


static struct miscdevice globalfifo_misc = {
    .minor  = GLOBALFIFO_MINOR,
    .name   = "globalfifo_misc",
    .fops   = &globalfifo_fops,
}; 

static int __init globalfifo_misc_init(void)
{
    int result;
    
    if (misc_register(&globalfifo_misc)) {    
		printk (KERN_WARNING "globalfifo: Couldn't register device 10, "
				"%d.\n", GLOBALFIFO_MINOR);
		return -EBUSY;
    }

    /* Allocating globalmem_dev structure dynamically */
    globalfifo_devp = kmalloc(sizeof(struct globalfifo_dev), GFP_KERNEL);
    if (!globalfifo_devp) {
        result = -ENOMEM;
        goto fail_malloc;
    }
    
    memset(globalfifo_devp, 0, sizeof(struct globalfifo_dev));

    /* Initialize semaphore structure */
    sema_init(&globalfifo_devp->sem, 1);

    /* Initialize wait_queue for read_wait_queue and write_wait_queue */
    init_waitqueue_head(&globalfifo_devp->r_wait);
    init_waitqueue_head(&globalfifo_devp->w_wait);

    return 0;

fail_malloc:
    misc_deregister(&globalfifo_misc);
    return result;
}

static void __exit globalfifo_misc_exit(void)
{    
    kfree(globalfifo_devp);
    misc_deregister(&globalfifo_misc);
}

module_init(globalfifo_misc_init);
module_exit(globalfifo_misc_exit);

MODULE_LICENSE("GPL");


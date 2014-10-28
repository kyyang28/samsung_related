

#include <linux/module.h>  
#include <linux/kernel.h>
#include <linux/init.h> 
#include <linux/fs.h>   
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>             /* kmalloc/kfree */
#include <linux/interrupt.h>        /* request_irq */
#include <linux/irq.h>              /* IRQ_TYPE_EDGE_BOTH */
#include <linux/sched.h>            /* TASK_INTERRUPTIBLE */
#include <linux/poll.h>             /* poll_wait */
#include <linux/miscdevice.h>       /* miscdevice, etc */
#include <asm/uaccess.h>
#include <asm/io.h>  
#include <asm/irq.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <mach/map.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pete&Charles <http://embeddedfansclub.taobao.com>");


//#define MINI2440_DBG        printk
#define MINI2440_DBG(...)

#define MINI2440_BUTTONS_NAME     "mini2440_buttons"

struct button_irq_desc {
    int irq;
    int pin;
    int number;
    char *name;	
};

static struct button_irq_desc button_irqs [] = {
    {IRQ_EINT8,  S3C2410_GPG( 0), 0, "K1"},
    {IRQ_EINT11, S3C2410_GPG( 3), 1, "K2"},
    {IRQ_EINT13, S3C2410_GPG( 5), 2, "K3"},
    {IRQ_EINT14, S3C2410_GPG( 6), 3, "K4"},
    {IRQ_EINT15, S3C2410_GPG( 7), 4, "K5"},
    {IRQ_EINT19, S3C2410_GPG(11), 5, "K6"},
};

static volatile unsigned char key_values[] = {'0', '0', '0', '0', '0', '0'};

static DECLARE_WAIT_QUEUE_HEAD(button_waitq);
static volatile int ev_press = 0;

extern unsigned int s3c2410_gpio_getpin(unsigned int pin);


static irqreturn_t buttons_interrupt(int irq, void *dev_id)
{
	struct button_irq_desc *button_irqs = (struct button_irq_desc *)dev_id;
	int down;

    down = !s3c2410_gpio_getpin(button_irqs->pin);

    if (down != (key_values[button_irqs->number] & 1)) { 

	key_values[button_irqs->number] = '0' + down;
	
        ev_press = 1;
        wake_up_interruptible(&button_waitq);
    }
    
    return IRQ_RETVAL(IRQ_HANDLED);
}


static int mini2440_buttons_open(struct inode *inode, struct file *file)
{
    int i;
    int err = 0;
    
    for (i = 0; i < sizeof(button_irqs)/sizeof(button_irqs[0]); i++) {
    	if (button_irqs[i].irq < 0) {
    		continue;
    	}
        err = request_irq(button_irqs[i].irq, buttons_interrupt, IRQ_TYPE_EDGE_BOTH, 
                          button_irqs[i].name, (void *)&button_irqs[i]);
        if (err)
            break;
    }

    if (err) {
        i--;
        for (; i >= 0; i--) {
    	    if (button_irqs[i].irq < 0) {
    		    continue;
    	    }
    	    disable_irq(button_irqs[i].irq);
            free_irq(button_irqs[i].irq, (void *)&button_irqs[i]);
        }
        return -EBUSY;
    }

    ev_press = 1;
    
    return 0;
}


static int mini2440_buttons_close(struct inode *inode, struct file *file)
{
    int i;
    
    for (i = 0; i < sizeof(button_irqs)/sizeof(button_irqs[0]); i++) {
    	if (button_irqs[i].irq < 0) {
    	    continue;
    	}
    	free_irq(button_irqs[i].irq, (void *)&button_irqs[i]);
    }

    return 0;
}


static int mini2440_buttons_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
    unsigned long err;

    if (!ev_press) {
    	if (filp->f_flags & O_NONBLOCK) {
            return -EAGAIN;
    	}else {
            MINI2440_DBG("Waiting.........\n");
    	    wait_event_interruptible(button_waitq, ev_press);
    	}
    }
    
    ev_press = 0;

    MINI2440_DBG("[DRIVER]%c %c %c %c %c %c", key_values[0], key_values[1], 
                key_values[2], key_values[3], key_values[4], key_values[5]);

    err = copy_to_user((void *)buff, (const void *)(&key_values), min(sizeof(key_values), count));

    return err ? -EFAULT : min(sizeof(key_values), count);
}

static unsigned int mini2440_buttons_poll( struct file *file, struct poll_table_struct *wait)
{
    unsigned int mask = 0;
    poll_wait(file, &button_waitq, wait);
    if (ev_press)
        mask |= POLLIN | POLLRDNORM;
    return mask;
}


static struct file_operations mini2440_buttons_fops = {
    .owner   =   THIS_MODULE,
    .open    =   mini2440_buttons_open,
    .release =   mini2440_buttons_close, 
    .read    =   mini2440_buttons_read,
    .poll    =   mini2440_buttons_poll,
};

static struct miscdevice mini2440_buttons_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = MINI2440_BUTTONS_NAME,
	.fops = &mini2440_buttons_fops,
};

static int __init mini2440_buttons_init(void)
{
	int ret;

	ret = misc_register(&mini2440_buttons_misc);

	printk (MINI2440_BUTTONS_NAME" is initialized!\n");

	return ret;
}

static void __exit mini2440_buttons_exit(void)
{
	misc_deregister(&mini2440_buttons_misc);
}

module_init(mini2440_buttons_init);
module_exit(mini2440_buttons_exit);


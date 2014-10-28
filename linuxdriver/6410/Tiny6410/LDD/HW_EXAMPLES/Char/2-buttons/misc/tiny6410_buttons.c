

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
#include <mach/gpio-bank-n.h>
#include <mach/gpio-bank-l.h>
#include <mach/map.h>


//#define TINY6410_DBG        printk
#define TINY6410_DBG(...)

#define TINY6410_BUTTONS_NAME     "tiny6410_buttons"

struct button_irq_desc {
    int irq;
    int number;
    char *name;	
};

static struct button_irq_desc button_irqs [] = {
    {IRQ_EINT( 0), 0, "KEY0"},
    {IRQ_EINT( 1), 1, "KEY1"},
    {IRQ_EINT( 2), 2, "KEY2"},
    {IRQ_EINT( 3), 3, "KEY3"},
    {IRQ_EINT( 4), 4, "KEY4"},
    {IRQ_EINT( 5), 5, "KEY5"},
    {IRQ_EINT(19), 6, "KEY6"},
    {IRQ_EINT(20), 7, "KEY7"},
};

static volatile unsigned char key_values[] = {'0', '0', '0', '0', '0', '0', '0', '0'};

static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

static volatile int ev_press = 0;


static irqreturn_t buttons_interrupt(int irq, void *dev_id)
{
	struct button_irq_desc *button_irqs = (struct button_irq_desc *)dev_id;
	int down;
	int number;
	unsigned tmp;

	udelay(0);
	number = button_irqs->number;
	switch(number) {
	case 0: case 1: case 2: case 3: case 4: case 5:
		tmp = readl(S3C64XX_GPNDAT);
		down = !(tmp & (1<<number));
		break;
	case 6: case 7:
		tmp = readl(S3C64XX_GPLDAT);
		down = !(tmp & (1 << (number + 5)));
		break;
	default:
		down = 0;
	}

	if (down != (key_values[number] & 1)) {
		key_values[number] = '0' + down;

        	ev_press = 1;
        	wake_up_interruptible(&button_waitq);
    	}

    return IRQ_RETVAL(IRQ_HANDLED);
}


static int tiny6410_buttons_open(struct inode *inode, struct file *file)
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


static int tiny6410_buttons_close(struct inode *inode, struct file *file)
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


static int tiny6410_buttons_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
    unsigned long err;

    if (!ev_press) {
    	if (filp->f_flags & O_NONBLOCK) {
            return -EAGAIN;
    	}else {
            TINY6410_DBG("Waiting.........\n");
    	    wait_event_interruptible(button_waitq, ev_press);
    	}
    }
    
    ev_press = 0;

    TINY6410_DBG("[DRIVER]%c %c %c %c %c %c %c %c", key_values[0], key_values[1], key_values[2], key_values[3], 
                key_values[4], key_values[5], key_values[6], key_values[7]);

    err = copy_to_user((void *)buff, (const void *)(&key_values), min(sizeof(key_values), count));

    return err ? -EFAULT : min(sizeof(key_values), count);
}

static unsigned int tiny6410_buttons_poll( struct file *file, struct poll_table_struct *wait)
{
    unsigned int mask = 0;
    poll_wait(file, &button_waitq, wait);
    if (ev_press)
        mask |= POLLIN | POLLRDNORM;
    return mask;
}


static struct file_operations tiny6410_buttons_fops = {
    .owner   =   THIS_MODULE,
    .open    =   tiny6410_buttons_open,
    .release =   tiny6410_buttons_close, 
    .read    =   tiny6410_buttons_read,
    .poll    =   tiny6410_buttons_poll,
};

static struct miscdevice tiny6410_buttons_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = TINY6410_BUTTONS_NAME,
	.fops = &tiny6410_buttons_fops,
};

static int __init tiny6410_buttons_init(void)
{
	int ret;

	ret = misc_register(&tiny6410_buttons_misc);

	printk (TINY6410_BUTTONS_NAME" is initialized!\n");

	return ret;
}

static void __exit tiny6410_buttons_exit(void)
{
	misc_deregister(&tiny6410_buttons_misc);
}

module_init(tiny6410_buttons_init);
module_exit(tiny6410_buttons_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pete&Charles <http://embeddedfansclub.taobao.com>");


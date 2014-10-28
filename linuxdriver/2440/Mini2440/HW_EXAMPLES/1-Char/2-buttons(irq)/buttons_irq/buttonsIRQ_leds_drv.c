
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

static int buttons_major;
static int leds_major;
static struct class *buttons_cls;
static struct class *leds_cls;

/* GPIO for leds */
static volatile unsigned long *GPBCON = NULL;
static volatile unsigned long *GPBDAT = NULL;

/* GPIO for buttons */
static volatile unsigned long *GPGCON = NULL;
static volatile unsigned long *GPGDAT = NULL;

struct buttonsIRQ_desc {
	int number;
	int irqNum;
	int pinNum;
	char *name;
};

static struct buttonsIRQ_desc buttons_irq[] = {
#if 0
	{0, IRQ_EINT8,  S3C2410_GPG( 0), "key1"},
	{1, IRQ_EINT11, S3C2410_GPG( 3), "key2"},
	{2, IRQ_EINT13, S3C2410_GPG( 5), "key3"},
	{3, IRQ_EINT14, S3C2410_GPG( 6), "key4"},
	{4, IRQ_EINT15, S3C2410_GPG( 7), "key5"},
	{5, IRQ_EINT19, S3C2410_GPG(11), "key6"},
#else
	{0, IRQ_EINT8,  0,  "key1"},
	{1, IRQ_EINT11, 3,  "key2"},
	{2, IRQ_EINT13, 5,  "key3"},
	{3, IRQ_EINT14, 6,  "key4"},
	{4, IRQ_EINT15, 7,  "key5"},
	{5, IRQ_EINT19, 11, "key6"},
#endif
};

static volatile int buttons_value[] = {0, 0, 0, 0, 0, 0};

static volatile int event_press = 0;
static DECLARE_WAIT_QUEUE_HEAD(buttons_waitqueue);

//extern unsigned int s3c2410_gpio_getpin(unsigned int pin);

static int leds_open(struct inode *inode, struct file *filp)
{
	*GPBCON |= (1<<10 | 1<<12 | 1<<14 | 1<<16);
	*GPBCON &= ~(1<<11 | 1<<13 | 1<<15 | 1<<17);
	return 0;
}

static long leds_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	switch (arg) {
    case 1:
		if (cmd)
			*GPBDAT |= (1<<5);
		else
			*GPBDAT &= ~(1<<5);
		break;

    case 2:
		if (cmd)
			*GPBDAT |= (1<<6);
		else 
			*GPBDAT &= ~(1<<6);
		break;
	
    case 3:
		if (cmd)
			*GPBDAT |= (1<<7);
		else
			*GPBDAT &= ~(1<<7);
		break;

    case 4:
		if (cmd)
			*GPBDAT |= (1<<8);
		else
			*GPBDAT &= ~(1<<8);
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static irqreturn_t buttonsIRQ_handler(int irq, void *dev_id)
{
	struct buttonsIRQ_desc *buttonsIRQ = (struct buttonsIRQ_desc *)dev_id;
	int buttons_down;

	//buttons_down = !s3c2410_gpio_getpin(buttonsIRQ->pinNum);
	buttons_down = !(*GPGDAT & (1<<buttonsIRQ->pinNum));

	if (buttons_down != (buttons_value[buttonsIRQ->number] & 1)) {
		buttons_value[buttonsIRQ->number] = buttons_down;
		event_press = 1;
		wake_up_interruptible(&buttons_waitqueue);
	}

	return IRQ_RETVAL(IRQ_HANDLED);
}

static int buttonsIRQ_open(struct inode *inode, struct file *filp)
{
	int i;
	
	for (i = 0; i < sizeof(buttons_irq)/sizeof(buttons_irq[0]); i++) {
		request_irq(buttons_irq[i].irqNum, buttonsIRQ_handler, IRQ_TYPE_EDGE_BOTH, 
						  buttons_irq[i].name, (void *)&buttons_irq[i]);
	}

	return 0;
}

static ssize_t buttonsIRQ_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
	wait_event_interruptible(buttons_waitqueue, event_press);

	event_press = 0;

	copy_to_user(buf, &buttons_value, min(sizeof(buttons_value), count));

	return min(sizeof(buttons_value), count);
}

static int buttonsIRQ_release(struct inode *inode, struct file *filp)
{
	int i;

	for (i = 0; i < sizeof(buttons_irq)/sizeof(buttons_irq[0]); i++) {
		free_irq(buttons_irq[i].irqNum, (void *)&buttons_irq[i]);
	}

	return 0;
}

static const struct file_operations buttonsIRQ_fops = {
	.owner		= THIS_MODULE,
	.open		= buttonsIRQ_open,
	.read		= buttonsIRQ_read,
	.release	= buttonsIRQ_release,
};

static const struct file_operations leds_fops = {
	.owner				= THIS_MODULE,
	.open				= leds_open,
	.unlocked_ioctl		= leds_ioctl,
};

static int __init buttonsIRQ_leds_drv_init(void)
{
	/* Register char device for buttons */
	buttons_major = register_chrdev(0, "buttonsName", &buttonsIRQ_fops);

	/* Register char device for leds */
	leds_major = register_chrdev(0, "ledsName", &leds_fops);

	/* class_create/device_create for buttons */
	buttons_cls = class_create(THIS_MODULE, "buttonsClass");
	device_create(buttons_cls, NULL, MKDEV(buttons_major, 0), NULL, "buttonsIRQ");	/* /dev/buttonsIRQ */

	/* class_create/device_create for leds */
	leds_cls = class_create(THIS_MODULE, "ledsClass");
	device_create(leds_cls, NULL, MKDEV(leds_major, 0), NULL, "leds");	/* /dev/leds */

	/* Change the PA of leds GPIO to VA (ioremap) */
	GPBCON = (volatile unsigned long *)ioremap(0x56000010, 8);
	GPBDAT = GPBCON + 1;

	GPGCON = (volatile unsigned long *)ioremap(0x56000060, 8);
	GPGDAT = GPGCON + 1;

    printk("###### Buttons driver is inserted into the kernel as a module! ######\n");
    printk("###### Leds driver is inserted into the kernel as a module! ######\n");
	
	return 0;
}

static void __exit buttonsIRQ_leds_drv_exit(void)
{
	/* iounmap for buttons GPIO */
	iounmap(GPGCON);
	
	/* iounmap for leds GPIO */
	iounmap(GPBCON);
	
	/* device destroy for leds */
	device_destroy(leds_cls, MKDEV(leds_major, 0));

	/* class destroy for leds */
	class_destroy(leds_cls);

	/* device destroy for buttons */
	device_destroy(buttons_cls, MKDEV(buttons_major, 0));

	/* class destroy for buttons */
	class_destroy(buttons_cls);

	/* unregister char device for leds */
	unregister_chrdev(leds_major, "ledsName");
    
	/* unregister char device for buttons */
	unregister_chrdev(buttons_major, "buttonsName");
    
	printk("###### Buttons driver is removed from kernel as a module! ######\n");
	printk("###### Leds driver is removed from kernel as a module! ######\n");
}

module_init(buttonsIRQ_leds_drv_init);
module_exit(buttonsIRQ_leds_drv_exit);

MODULE_LICENSE("GPL");


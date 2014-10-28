
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/pci.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <asm/unistd.h>

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

static int buttonsPoll_open(struct inode *inode, struct file *filp)
{
	/* GPIO setup for buttons */
	*GPGCON = (0x0<<0) | (0x0<<6) | (0x0<<10) | (0x0<<12) | (0x0<<14) | (0x0<<22);
	return 0;
}

static ssize_t buttonsPoll_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
	unsigned char kbuf[6];

	memset(kbuf, 0, sizeof(kbuf));

	kbuf[0] = (*GPGDAT & (1<<0))  ? 1 : 0;	/* key1 */
	kbuf[1] = (*GPGDAT & (1<<3))  ? 1 : 0;	/* key2 */
	kbuf[2] = (*GPGDAT & (1<<5))  ? 1 : 0;	/* key3 */
	kbuf[3] = (*GPGDAT & (1<<6))  ? 1 : 0;	/* key4 */
	kbuf[4] = (*GPGDAT & (1<<7))  ? 1 : 0;	/* key5 */
	kbuf[5] = (*GPGDAT & (1<<11)) ? 1 : 0;	/* key6 */

	copy_to_user(buf, kbuf, size);

	return sizeof(kbuf);
}

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

static const struct file_operations buttonsPoll_fops = {
	.owner		= THIS_MODULE,
	.open		= buttonsPoll_open,
	.read		= buttonsPoll_read,
};

static const struct file_operations leds_fops = {
	.owner				= THIS_MODULE,
	.open				= leds_open,
	.unlocked_ioctl		= leds_ioctl,
};

static int __init buttonsPoll_leds_drv_init(void)
{
	/* Register char device for buttons */
	buttons_major = register_chrdev(0, "buttonsName", &buttonsPoll_fops);

	/* Register char device for leds */
	leds_major = register_chrdev(0, "ledsName", &leds_fops);

	/* class_create/device_create for buttons */
	buttons_cls = class_create(THIS_MODULE, "buttonsClass");
	device_create(buttons_cls, NULL, MKDEV(buttons_major, 0), NULL, "buttonsPoll");	/* /dev/buttonsPoll */

	/* class_create/device_create for leds */
	leds_cls = class_create(THIS_MODULE, "ledsClass");
	device_create(leds_cls, NULL, MKDEV(leds_major, 0), NULL, "leds");	/* /dev/buttons */

	/* Change the PA of buttons GPIO to VA (ioremap) */
	GPGCON = (volatile unsigned long *)ioremap(0x56000060, 8);
	GPGDAT = GPGCON + 1;

	/* Change the PA of leds GPIO to VA (ioremap) */
	GPBCON = (volatile unsigned long *)ioremap(0x56000010, 8);
	GPBDAT = GPBCON + 1;

    printk("###### Buttons driver is inserted into the kernel as a module! ######\n");
    printk("###### Leds driver is inserted into the kernel as a module! ######\n");
	
	return 0;
}

static void __exit buttonsPoll_leds_drv_exit(void)
{
	/* iounmap for leds GPIO */
	iounmap(GPBCON);
	
	/* iounmap for buttons GPIO */
	iounmap(GPGCON);

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

module_init(buttonsPoll_leds_drv_init);
module_exit(buttonsPoll_leds_drv_exit);

MODULE_LICENSE("GPL");


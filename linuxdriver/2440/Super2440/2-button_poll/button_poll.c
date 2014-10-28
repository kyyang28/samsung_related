
#include <linux/module.h>  
#include <linux/kernel.h>
#include <linux/init.h> 
#include <linux/fs.h>   
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <asm/io.h>  
#include <asm/irq.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>


int major;
static struct class *button_poll_class;
static struct device *button_poll_dev;

static volatile unsigned long *rGPFCON = NULL;
static volatile unsigned long *rGPFDAT = NULL;


static int button_poll_open(struct inode *inode, struct file *file)
{
	/* 
	 *	GPF1  [3:2] 		key 2 
   	 *	GPF3  [7:6]			Key 3
   	 *	GPF5  [11:10]		Key 4
   	 *	GPF0  [1:0]			Key 5
   	 *	GPF2  [5:4]			Key 6
   	 *	GPF4  [9:8]			Key 7
   	 *	00 = Input
	 */
	*rGPFCON &= ~((0x3) | (0x3<<2) | (0x3<<4) | (0x3<<6) | (0x3<<8) | (0x3<<10));
	
	return 0;
}


static ssize_t button_poll_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	/* Return the button level value */
	unsigned char key_vals[6];
	int regval;

	if(size != sizeof(key_vals))
		return -EINVAL;

	regval = *rGPFDAT;
	key_vals[0] = (regval & 1<<1) ? 1 : 0;
	key_vals[1] = (regval & 1<<3) ? 1 : 0;
	key_vals[2] = (regval & 1<<5) ? 1 : 0;
	key_vals[3] = (regval & 1<<0) ? 1 : 0;
	key_vals[4] = (regval & 1<<2) ? 1 : 0;
	key_vals[5] = (regval & 1<<4) ? 1 : 0;

	copy_to_user(buf, key_vals, sizeof(key_vals));
	
	return sizeof(key_vals);
}


static struct file_operations button_poll_fops = {
	.owner = THIS_MODULE,
	.open = button_poll_open,
	.read = button_poll_read,
};


static int button_poll_init(void)
{
	if((major = register_chrdev(0, "button_poll", &button_poll_fops)) < 0) {
		printk(KERN_ERR "unable to register major device number %d\n", major);
		return -EIO;
	}

	button_poll_class = class_create(THIS_MODULE, "button_poll");
	if(IS_ERR(button_poll_class))
		return PTR_ERR(button_poll_class);

	button_poll_dev = device_create(button_poll_class, NULL, MKDEV(major, 0), NULL, "button_poll", 0); 
	if(IS_ERR(button_poll_dev))
		return PTR_ERR(button_poll_dev);

	rGPFCON = (volatile unsigned long *)ioremap(0x56000050, 16);
	rGPFDAT = rGPFCON + 1;
	
	return 0;
}


static void button_poll_exit(void)
{
	unregister_chrdev(major, "button_poll");
	device_destroy(button_poll_class, MKDEV(major, 0));
	class_destroy(button_poll_class);
	iounmap(rGPFCON);
}

module_init(button_poll_init);
module_exit(button_poll_exit);

MODULE_LICENSE("GPL");



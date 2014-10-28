
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
static struct class *leddrv_class;

static volatile unsigned long *rGPFCON = NULL;
static volatile unsigned long *rGPFDAT = NULL;
static volatile unsigned long *rGPGCON = NULL;
static volatile unsigned long *rGPGDAT = NULL;

static int leddrv_open(struct inode *inode, struct file *file)
{
	/* Configure GPF6, GPG0, GPG1, GPG10 */
	/* GPF6 LED1  [13:12]   01 = Output */
	*rGPFCON &= ~((0x3<<12));
	*rGPFCON |= (0x1<<12);
                            
    /* 
     *	GPG0 LED2    [1:0]       01 = Output
     *	GPG1 LED3    [3:2]       01 = Output
     *  GPG10 LED4   [21:20]     01 = Output
     */
    *rGPGCON &= ~((0x3) | (0x3<<2) | (0x3<<20));
	*rGPGCON |= ((0x1) | (0x1<<2) | (0x1<<20));

	return 0;
}


static ssize_t leddrv_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	int val;

	copy_from_user(&val, buf, count);

	if(val == 1) {
		/* light off */
		*rGPFDAT |= (1<<6);
		*rGPGDAT |= (1<<0 | 1<<1 | 1<<10);
	}else {
		/* light on */
		*rGPFDAT &= ~(1<<6);
		*rGPGDAT &= ~(1<<0 | 1<<1 | 1<<10);
	}
	return 0;
}


static struct file_operations leddrv_fops = {
	.owner = THIS_MODULE,
	.open = leddrv_open,
	.write = leddrv_write,
};


static int leddrv_init(void)
{
	major = register_chrdev(0, "leddrv", &leddrv_fops);

	leddrv_class = class_create(THIS_MODULE, "leddrv");
	if(IS_ERR(leddrv_class))
		return PTR_ERR(leddrv_class);

	device_create(leddrv_class, NULL, MKDEV(major, 0), NULL, "led");

	rGPFCON = (volatile unsigned long *)ioremap(0x56000050, 16);
	rGPFDAT = rGPFCON + 1;
	rGPGCON = (volatile unsigned long *)ioremap(0x56000060, 12);
	rGPGDAT = rGPGCON + 1;
	
	return 0;
}

static void leddrv_exit(void)
{
	unregister_chrdev(major, "leddrv");
	device_destroy(leddrv_class, MKDEV(major, 0));
	class_destroy(leddrv_class);
	iounmap(rGPFCON);
	iounmap(rGPGCON);
}

module_init(leddrv_init);
module_exit(leddrv_exit);

MODULE_LICENSE("GPL");


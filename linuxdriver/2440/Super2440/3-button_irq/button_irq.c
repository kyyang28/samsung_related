
#include <linux/module.h>  
#include <linux/kernel.h>
#include <linux/init.h> 
#include <linux/fs.h>   
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/gpio.h>

#include <asm/uaccess.h>
#include <asm/io.h>  
#include <asm/irq.h>

#include <mach/regs-gpio.h>
#include <mach/hardware.h>





static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

/* 中断事件标志, 中断服务程序将它置1，button_irq_read将它清0 */
static volatile int ev_press = 0;



int major;
static struct class *button_irq_class;
static struct device *button_irq_dev;

//static volatile unsigned long *rGPFCON = NULL;
//static volatile unsigned long *rGPFDAT = NULL;


struct pin_desc {
	unsigned int pin;
	unsigned int key_val;
};


struct pin_desc pins_desc[6] = {
	{ S3C2410_GPF(0), 0x04},  	// K5
	{ S3C2410_GPF(1), 0x01},  	// K2
	{ S3C2410_GPF(2), 0x05}, 	// K6
	{ S3C2410_GPF(3), 0x02},  	// K3
	{ S3C2410_GPF(4), 0x06}, 	// K7
	{ S3C2410_GPF(5), 0x03},  	// K4	
};

static unsigned char keyval;

static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	struct pin_desc *pindesc = (struct pin_desc *)dev_id;
	unsigned int pinval;

	//printk("pindesc->pin = %d\n", pindesc->pin);
	
	pinval = s3c2410_gpio_getpin(pindesc->pin);

	//printk("pinval = %d\n", pinval);
	//printk("pindesc->key_val = %x\n", pindesc->key_val);

	if(pinval) {
		/* key is not pressed down */
		keyval = 0x80 | pindesc->key_val;
	}else {
		/* key is pressed down */
		keyval = pindesc->key_val;
	}

	ev_press = 1;                  			/* 表示中断发生了 */
    wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */

	
	return IRQ_RETVAL(IRQ_HANDLED);
}


static int button_irq_open(struct inode *inode, struct file *file)
{
	/* request_irq will set irq mode automatically */
	request_irq(IRQ_EINT1, buttons_irq, IRQ_TYPE_EDGE_BOTH, "K2", &pins_desc[1]);
	request_irq(IRQ_EINT3, buttons_irq, IRQ_TYPE_EDGE_BOTH, "K3", &pins_desc[3]);
	request_irq(IRQ_EINT5, buttons_irq, IRQ_TYPE_EDGE_BOTH, "K4", &pins_desc[5]);
	request_irq(IRQ_EINT0, buttons_irq, IRQ_TYPE_EDGE_BOTH, "K5", &pins_desc[0]);
	request_irq(IRQ_EINT2, buttons_irq, IRQ_TYPE_EDGE_BOTH, "K6", &pins_desc[2]);
	request_irq(IRQ_EINT4, buttons_irq, IRQ_TYPE_EDGE_BOTH, "K7", &pins_desc[4]);
	
	return 0;
}


static ssize_t button_irq_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	if(size != 1)
		return -EINVAL;

	/* if no interrupt occurs, wait */
	wait_event_interruptible(button_waitq, ev_press);

	/* if interrupt occurs, copy keyval to user in user space */
	copy_to_user(buf, &keyval, 1);
	ev_press = 0;
	
	return 1;
}


static int button_irq_release(struct inode *inode, struct file *file) 
{
	free_irq(IRQ_EINT1, &pins_desc[1]);	// K2
	free_irq(IRQ_EINT3, &pins_desc[3]);	// K3
	free_irq(IRQ_EINT5, &pins_desc[5]);	// K4
	free_irq(IRQ_EINT0, &pins_desc[0]);	// K5
	free_irq(IRQ_EINT2, &pins_desc[2]);	// K6
	free_irq(IRQ_EINT4, &pins_desc[4]);	// K7
	
	return 0;
}


static struct file_operations button_irq_fops = {
	.owner   = THIS_MODULE,
	.open    = button_irq_open,
	.read    = button_irq_read,
	.release = button_irq_release,
};


static int button_irq_init(void)
{
	if((major = register_chrdev(0, "button_irq", &button_irq_fops)) < 0) {
		printk(KERN_ERR "unable to register major device number %d\n", major);
		return -EIO;
	}

	button_irq_class = class_create(THIS_MODULE, "button_irq");
	if(IS_ERR(button_irq_class))
		return PTR_ERR(button_irq_class);

	button_irq_dev = device_create(button_irq_class, NULL, MKDEV(major, 0), NULL, "button_irq"); 
	if(IS_ERR(button_irq_dev))
		return PTR_ERR(button_irq_dev);

	//rGPFCON = (volatile unsigned long *)ioremap(0x56000050, 16);
	//rGPFDAT = rGPFCON + 1;
	
	return 0;
}


static void button_irq_exit(void)
{
	unregister_chrdev(major, "button_irq");
	device_destroy(button_irq_class, MKDEV(major, 0));
	class_destroy(button_irq_class);
	//iounmap(rGPFCON);
}

module_init(button_irq_init);
module_exit(button_irq_exit);

MODULE_LICENSE("GPL");



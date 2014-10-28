
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
#include <linux/poll.h>
#include <linux/types.h>

#include <asm/uaccess.h>
#include <asm/io.h>  
#include <asm/irq.h>


#define KERNEL_RW_R8            0
#define KERNEL_RW_R16           1
#define KERNEL_RW_R32           2

#define KERNEL_RW_W8            3
#define KERNEL_RW_W16           4
#define KERNEL_RW_W32           5

#define DBG_PRINTK              printk
//#define DBG_PRINTK(x,...)


static int major;
static struct class *kernel_rw_class;
static struct device *kernel_rw_dev;

static long kernel_rw_ioctl(struct file *file, unsigned int cmd, unsigned long args)
{
    volatile unsigned char      *p8;
    volatile unsigned short     *p16;
    volatile unsigned int       *p32;

    unsigned int val, addr;
    unsigned int buf[2];

    copy_from_user(buf, (void __user *)args, 8);
    addr = buf[0];
    val  = buf[1];

    p8  = (volatile unsigned char *)ioremap(addr, 4);
    //p16 = (volatile unsigned short *)p8;
    //p32 = (volatile unsigned int *)p8;
    p16 = (volatile unsigned short *)ioremap(addr, 4);
    p32 = (volatile unsigned int *)ioremap(addr, 4);
    
    switch (cmd) {

        case KERNEL_RW_R8:
            val = *p8;
            copy_to_user((void __user *)(args+4), &val, 4);
            break;
            
        case KERNEL_RW_R16:            
            val = *p16;
            DBG_PRINTK("Charles.Y[kernel]: r16\n");
            copy_to_user((void __user *)(args+4), &val, 4);
            break;

        case KERNEL_RW_R32:
            val = *p32;
            DBG_PRINTK("Charles.Y[kernel]: r32\n");
            copy_to_user((void __user *)(args+4), &val, 4);
            break;

        case KERNEL_RW_W8:
            *p8 = val;
            break;

        case KERNEL_RW_W16:
            *p16 = val;
            break;

        case KERNEL_RW_W32:
            *p32 = val;
            break;    
    }

    iounmap(p8);
    iounmap(p16);
    iounmap(p32);
    
    return 0;
}

static struct file_operations kernel_rw_fops = {
    .owner          =   THIS_MODULE,
    .unlocked_ioctl =   kernel_rw_ioctl,
};

static int __init kernel_rw_init(void)
{
    DBG_PRINTK("Charles.Y: kernel_rw_init\n");
    major = register_chrdev(0, "kernel_rw", &kernel_rw_fops);

	kernel_rw_class = class_create(THIS_MODULE, "kernel_rw");
	if(IS_ERR(kernel_rw_class))
		return PTR_ERR(kernel_rw_class);

	kernel_rw_dev = device_create(kernel_rw_class, NULL, MKDEV(major, 0), NULL, "kernelRW"); 
	if(IS_ERR(kernel_rw_dev))
		return PTR_ERR(kernel_rw_dev);
    
    return 0;
}

static void __exit kernel_rw_exit(void)
{
    DBG_PRINTK("Charles.Y: kernel_rw_exit\n");
    unregister_chrdev(major, "kernel_rw");
	device_destroy(kernel_rw_class, MKDEV(major, 0));
	class_destroy(kernel_rw_class);
}

module_init(kernel_rw_init);
module_exit(kernel_rw_exit);

MODULE_LICENSE("Dual BSD/GPL");


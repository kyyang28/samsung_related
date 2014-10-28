
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
#include <linux/poll.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>

#include <asm/uaccess.h>
#include <asm/io.h>  
#include <asm/irq.h>

#include <mach/regs-gpio.h>
#include <mach/hardware.h>

#define MEM_CPY_NO_DMA      (0)
#define MEM_CPY_DMA         (1)
#define BUF_SIZE            (512*1024)

static int major = 0;

static char *src;
static u32 src_phys;
static char *dst;
static u32 dst_phys;

static struct class *s3c_dma_cls;
static struct device *s3c_dma_dev;

static long s3c_dma_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
        case MEM_CPY_NO_DMA: 
            break;

        case MEM_CPY_DMA: 
            break;
    }
    
    return 0;
}

static struct file_operations s3c_dma_fops = {
    .owner              = THIS_MODULE,
    .unlocked_ioctl     = s3c_dma_ioctl,
};

static int __init s3c_dma_init(void)
{

    /* Allocate SRC, DST buffers, WARNING: DO NOT use kmalloc to allocate */
    src = dma_alloc_writecombine(NULL, BUF_SIZE, &src_phys, GFP_KERNEL);
    if (NULL == src) {
        printk("dma_alloc_writecombine for src failed\n");
        return -ENOMEM;
    }
    
    dst = dma_alloc_writecombine(NULL, BUF_SIZE, &dst_phys, GFP_KERNEL);
    if (NULL == dst) {
        dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
        printk("dma_alloc_writecombine for dst failed\n");
        return -ENOMEM;
    }

	if ((major = register_chrdev(0, "s3c_dma", &s3c_dma_fops)) < 0) {
		printk(KERN_ERR "unable to register major device number %d\n", major);
		return -EIO;
	}

	s3c_dma_cls = class_create(THIS_MODULE, "s3c_dma");
	if(IS_ERR(s3c_dma_cls))
		return PTR_ERR(s3c_dma_cls);

	s3c_dma_dev = device_create(s3c_dma_cls, NULL, MKDEV(major, 0), NULL, "dma"); 
	if(IS_ERR(s3c_dma_dev))
		return PTR_ERR(s3c_dma_dev);
        
    return 0;
}

static void __exit s3c_dma_exit(void)
{
    device_destroy(s3c_dma_cls, MKDEV(major, 0));
    class_destroy(s3c_dma_cls);
    unregister_chrdev(major, "s3c_dma");
    dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
    dma_free_writecombine(NULL, BUF_SIZE, dst, dst_phys);
}

module_init(s3c_dma_init);
module_exit(s3c_dma_exit);

MODULE_LICENSE("Dual BSD/GPL");


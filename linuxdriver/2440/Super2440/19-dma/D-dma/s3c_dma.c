

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


#define MEM_CPY_NO_DMA              (0)
#define MEM_CPY_DMA                 (1)
#define BUFF_SIZE                   (512*1024)

#define DMA0_BASE_ADDRESS           (0x4B000000)
#define DMA1_BASE_ADDRESS           (0x4B000040)
#define DMA2_BASE_ADDRESS           (0x4B000080)
#define DMA3_BASE_ADDRESS           (0x4B0000C0)


struct s3c_dma_regs {
    unsigned long DISRC; 
    unsigned long DISRCC; 
    unsigned long DIDST; 
    unsigned long DIDSTC; 
    unsigned long DCON; 
    unsigned long DSTAT; 
    unsigned long DCSRC; 
    unsigned long DCDST; 
    unsigned long DMASKTRIG; 
};


static int major = 0;

static char *src;
static u32 src_phys;
static char *dst;
static u32 dst_phys;

static struct class *s3c_dma_cls;
static struct device *s3c_dma_dev;

static volatile struct s3c_dma_regs *dma_regs;

static DECLARE_WAIT_QUEUE_HEAD(s3c_dma_waitq);
static volatile int ev_dma;

static long s3c_dma_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int i;

    memset(src, 0xAA, BUFF_SIZE);
    memset(dst, 0xCC, BUFF_SIZE);
    
    switch (cmd) {
        case MEM_CPY_NO_DMA: 
            for (i = 0; i < BUFF_SIZE; ++i)
                dst[i] = src[i];
            
            if (memcmp(src, dst, BUFF_SIZE) == 0)
                printk("MEM_CPY_NO_DMA is fine\n");
            else
                printk("MEM_CPY_NO_DMA is error\n");
                
            break;
        case MEM_CPY_DMA: 
            ev_dma = 0;
            dma_regs->DISRC         = src_phys;
            dma_regs->DISRCC        = ((0<<1) | (0<<0));
            dma_regs->DIDST         = dst_phys;
            dma_regs->DIDSTC        = ((0<<2) | (0<<1) | (0<<0));
            dma_regs->DCON          = ((1<<30) | (1<<29) | (0<<28) | (1<<27) | (0<<23) | (0<<20) | (BUFF_SIZE));

            /* Trigger the DMA channel 0 */
            dma_regs->DMASKTRIG     = ((1<<1) | (1<<0));

            /* Sleep */
            wait_event_interruptible(s3c_dma_waitq, ev_dma);
            
            if (memcmp(src, dst, BUFF_SIZE) == 0)
                printk("MEM_CPY_DMA is fine\n");
            else
                printk("MEM_CPY_DMA is error\n");

            break;
    }
    
    return 0;
}

static irqreturn_t s3c_dma_irq(int irq, void *dev_id)
{
    ev_dma = 1;
    wake_up_interruptible(&s3c_dma_waitq);
    return IRQ_HANDLED;
}

static struct file_operations s3c_dma_fops = {
    .owner              = THIS_MODULE,
    .unlocked_ioctl     = s3c_dma_ioctl,
};

static int __init s3c_dma_init(void)
{

    /* Allocate SRC, DST buffers, WARNING: DO NOT use kmalloc to allocate */
    src = dma_alloc_writecombine(NULL, BUFF_SIZE, &src_phys, GFP_KERNEL);
    if (NULL == src) {
        printk("dma_alloc_writecombine for src failed\n");
        return -ENOMEM;
    }
    
    dst = dma_alloc_writecombine(NULL, BUFF_SIZE, &dst_phys, GFP_KERNEL);
    if (NULL == dst) {
        dma_free_writecombine(NULL, BUFF_SIZE, src, src_phys);
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

    dma_regs = ioremap(DMA0_BASE_ADDRESS, sizeof(struct s3c_dma_regs));
    if (NULL== dma_regs) {
        printk(KERN_ERR "s3c_dma: ioremap failed\n");        
        device_destroy(s3c_dma_cls, MKDEV(major, 0));
        class_destroy(s3c_dma_cls);
        unregister_chrdev(major, "s3c_dma");
        dma_free_writecombine(NULL, BUFF_SIZE, src, src_phys);
        dma_free_writecombine(NULL, BUFF_SIZE, dst, dst_phys);
        return -EIO;
    }

	if (request_irq(IRQ_DMA0, s3c_dma_irq, 0, "s3c_dma", NULL) < 0) {
		printk(KERN_ERR "s3c_dma: interrupt request failed.\n");        
        iounmap(dma_regs);
        device_destroy(s3c_dma_cls, MKDEV(major, 0));
        class_destroy(s3c_dma_cls);
        unregister_chrdev(major, "s3c_dma");
        dma_free_writecombine(NULL, BUFF_SIZE, src, src_phys);
        dma_free_writecombine(NULL, BUFF_SIZE, dst, dst_phys);
		return -EBUSY;
	}
        
    return 0;
}

static void __exit s3c_dma_exit(void)
{
    free_irq(IRQ_DMA0, NULL);
    iounmap(dma_regs);
    device_destroy(s3c_dma_cls, MKDEV(major, 0));
    class_destroy(s3c_dma_cls);
    unregister_chrdev(major, "s3c_dma");
    dma_free_writecombine(NULL, BUFF_SIZE, src, src_phys);
    dma_free_writecombine(NULL, BUFF_SIZE, dst, dst_phys);
}

module_init(s3c_dma_init);
module_exit(s3c_dma_exit);

MODULE_LICENSE("Dual BSD/GPL");



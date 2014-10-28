

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


struct s3c_dma0_regs {
    unsigned long DISRC0; 
    unsigned long DISRCC0; 
    unsigned long DIDST0; 
    unsigned long DIDSTC0; 
    unsigned long DCON0; 
    unsigned long DSTAT0; 
    unsigned long DCSRC0; 
    unsigned long DCDST0; 
    unsigned long DMASKTRIG0; 
};

struct s3c_dma1_regs {
    unsigned long DISRC1; 
    unsigned long DISRCC1; 
    unsigned long DIDST1; 
    unsigned long DIDSTC1; 
    unsigned long DCON1; 
    unsigned long DSTAT1; 
    unsigned long DCSRC1; 
    unsigned long DCDST1; 
    unsigned long DMASKTRIG1; 
};

struct s3c_dma2_regs {
    unsigned long DISRC2; 
    unsigned long DISRCC2; 
    unsigned long DIDST2; 
    unsigned long DIDSTC2; 
    unsigned long DCON2; 
    unsigned long DSTAT2; 
    unsigned long DCSRC2; 
    unsigned long DCDST2; 
    unsigned long DMASKTRIG2; 
};

struct s3c_dma3_regs {
    unsigned long DISRC3; 
    unsigned long DISRCC3; 
    unsigned long DIDST3; 
    unsigned long DIDSTC3; 
    unsigned long DCON3; 
    unsigned long DSTAT3; 
    unsigned long DCSRC3; 
    unsigned long DCDST3; 
    unsigned long DMASKTRIG3; 
};

static int major = 0;

static char *src;
static u32 src_phys;
static char *dst;
static u32 dst_phys;

static struct class *s3c_dma_cls;
static struct device *s3c_dma_dev;

static struct s3c_dma0_regs *dma0_regs;


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
                printk("MEM_CPY_NO_DMA ok\n");
            else
                printk("MEM_CPY_DMA error\n");
                
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

    dma0_regs = ioremap(DMA0_BASE_ADDRESS, sizeof(struct s3c_dma0_regs));
    if (NULL== dma0_regs) {
        printk(KERN_ERR "s3c_dma: ioremap failed\n");        
        device_destroy(s3c_dma_cls, MKDEV(major, 0));
        class_destroy(s3c_dma_cls);
        unregister_chrdev(major, "s3c_dma");
        dma_free_writecombine(NULL, BUFF_SIZE, src, src_phys);
        dma_free_writecombine(NULL, BUFF_SIZE, dst, dst_phys);
        return -EIO;
    }
        
    return 0;
}

static void __exit s3c_dma_exit(void)
{
    iounmap(dma0_regs);
    device_destroy(s3c_dma_cls, MKDEV(major, 0));
    class_destroy(s3c_dma_cls);
    unregister_chrdev(major, "s3c_dma");
    dma_free_writecombine(NULL, BUFF_SIZE, src, src_phys);
    dma_free_writecombine(NULL, BUFF_SIZE, dst, dst_phys);
}

module_init(s3c_dma_init);
module_exit(s3c_dma_exit);

MODULE_LICENSE("Dual BSD/GPL");



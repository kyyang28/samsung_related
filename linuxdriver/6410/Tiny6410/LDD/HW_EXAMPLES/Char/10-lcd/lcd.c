#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>

#include <asm/uaccess.h>
#include <asm/setup.h>
#include <asm/system.h>
#include <linux/fb.h>
#include <linux/module.h>
#include <asm/pgtable.h>

//static struct fb_info *s3c6410_lcd;


//static unsigned int   pseudo_pal[16];
static unsigned int gpio_base;  /* 0x7F008000 */

static volatile unsigned long * mifpcon; //    (*((volatile unsigned long *)0x7410800c))
static volatile unsigned long * spcon; //         (*((volatile unsigned long *)0x7f0081a0))

#define GPECON  (*((volatile unsigned long *)(gpio_base + 0x80)))
#define GPEDAT  (*((volatile unsigned long *)(gpio_base + 0x84)))
#define GPFCON  (*((volatile unsigned long *)(gpio_base + 0xA0)))
#define GPFDAT  (*((volatile unsigned long *)(gpio_base + 0xA4)))
#define GPICON  (*((volatile unsigned long *)(gpio_base + 0x100)))
#define GPJCON  (*((volatile unsigned long *)(gpio_base + 0x120)))

struct s3c_display_regs {
	unsigned long vidcon0;
	unsigned long vidcon1;
	unsigned long vidcon2;
	unsigned long reserver1;
	unsigned long vidtcon0;
	unsigned long vidtcon1;
	unsigned long vidtcon2;
	unsigned long reserver2;
	unsigned long wincon0;
	unsigned long wincon1;
	unsigned long wincon2;
	unsigned long wincon3;
	unsigned long wincon4;
	unsigned long reserver3[3];
	unsigned long vidosd0a;
	unsigned long vidosd0b;
	unsigned long vidosd0c;
	unsigned long reserver4;
	unsigned long vidosd1a;
	unsigned long vidosd1b;
	unsigned long vidosd1c;
	unsigned long vidosd1d;
	unsigned long vidosd2a;
	unsigned long vidosd2b;
	unsigned long vidosd2c;
	unsigned long vidosd2d;
	unsigned long vidosd3a;
	unsigned long vidosd3b;
	unsigned long vidosd3c;
	unsigned long reserver5;
	unsigned long vidosd4a;
	unsigned long vidosd4b;
	unsigned long vidosd4c;
	unsigned long reserver6[5];
	unsigned long vidw00add0b0;
	unsigned long vidw00add0b1;
	unsigned long vidw01add0b0;
	unsigned long vidw01add0b1;
	unsigned long vidw02add0;
	unsigned long reserver7;
	unsigned long vidw03add0;
	unsigned long reserver8;
	unsigned long vidw04add0;
	unsigned long reserver9[3];
	unsigned long vidw00add1b0;
	unsigned long vidw00add1b1;
};

static struct s3c_display_regs * lcd_regs;

static int major;
static struct class *cls;
static struct fb_var_screeninfo var;	/* Current var */
static struct fb_fix_screeninfo fix;	/* Current fix */

static unsigned long screen_base, screen_size;


static long mylcd_ioctl(struct file *filp, unsigned int cmd, unsigned long args)
{
	switch (cmd)
	{
		case FBIOGET_FSCREENINFO:
		{
			copy_to_user((void __user *)args, &fix, sizeof(fix));
			return 0;
			break;
		}
		case FBIOGET_VSCREENINFO:
		{
			copy_to_user((void __user *)args, &var, sizeof(var));
			return 0;
			break;
		}
		default:
		{
			return -EINVAL;
		}
	}
}


static int mylcd_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long size   = vma->vm_end - vma->vm_start;

	vma->vm_flags |= (VM_IO | VM_DONTEXPAND);
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	if (io_remap_pfn_range(vma, vma->vm_start, fix.smem_start >> PAGE_SHIFT,
			       size, vma->vm_page_prot))
		return -EAGAIN;

	return 0;
}

static struct file_operations mylcd_fops = {
	.owner              = THIS_MODULE,
	.unlocked_ioctl     = mylcd_ioctl,
	.mmap               = mylcd_mmap,
};

 
void s3c6410_lcd_hardware_init(void)
{
#define  VSPW         9
#define  VBPD          1
#define  LINEVAL     271
#define  VFPD          1


#define  HSPW         40    
#define  HBPD          1
#define  HOZVAL      479
#define  HFPD          1

#define LeftTopX     0
#define LeftTopY     0

#define RightBotX   479
#define RightBotY   271

	gpio_base = ioremap(0x7F008000, 1024);

	/* 1. configure GPIO */
	GPICON = 0xaaaaaaaa;  /* gpi0~gpi15 => lcd_vd[0~15] */
	GPJCON = 0xaaaaaaa;   /* gpj0~gpi11 =>lcd */
	GPFCON &= ~(0x3<<28);
	GPFCON |=  (1<<28);    /* GPF14 backlight */
	GPECON &= ~(0xf);
	GPECON |= (0x1);       /* GPE0: LCD on/off */

	/* 2.  */
	mifpcon = ioremap(0x7410800c, 4);
	spcon   = ioremap(0x7f0081a0, 4);
	*mifpcon &= ~(1<<3);   /* Normal mode */

	*spcon    &= ~(0x3);
	*spcon    |= 0x1;            /* RGB I/F style */

	/* 3. set s3c6410's display controller */	
	lcd_regs = ioremap(0x77100000, 4096);
	lcd_regs->vidcon0 &= ~((3<<26) | (3<<17) | (0xff<<6)  | (3<<2));     /* RGB I/F, RGB Parallel format,  */
	lcd_regs->vidcon0 |= ((14<<6) | (1<<4) );      /* vclk== HCLK / (CLKVAL+1) = 133/15 = 9MHz */

	lcd_regs->vidcon1 &= ~(1<<7);   /* \u5728vclk\u7684\u4e0b\u964d\u6cbf\u83b7\u53d6\u6570\u636e */
	lcd_regs->vidcon1 |= ((1<<6) | (1<<5));  /* HSYNC\u9ad8\u7535\u5e73\u6709\u6548, VSYNC\u9ad8\u7535\u5e73\u6709\u6548, */

	lcd_regs->vidtcon0 = (VBPD << 16) | (VFPD << 8) | (VSPW << 0);
	lcd_regs->vidtcon1 = (HBPD << 16) | (HFPD << 8) | (HSPW << 0);
	lcd_regs->vidtcon2 = (LINEVAL << 11) | (HOZVAL << 0);

	lcd_regs->wincon0 &= ~(0xf << 2);
	lcd_regs->wincon0 |= (0xb<<2);    /* unpacked 24 BPP (non-palletized R:8-G:8-B:8 ) */

	lcd_regs->vidosd0a = (LeftTopX<<11) | (LeftTopY << 0);
	lcd_regs->vidosd0b = (RightBotX<<11) | (RightBotY << 0);
	lcd_regs->vidosd0c = (LINEVAL + 1) * (HOZVAL + 1);

	lcd_regs->vidw00add0b0 = fix.smem_start;
	lcd_regs->vidw00add1b0 = (fix.smem_start + fix.smem_len) & 0xffffff;		
}

static void s3c6410_lcd_hardware_enable(void)
{
	/* 1. enable s3c6410 display controller */
	lcd_regs->vidcon0 |= 0x3;
	lcd_regs->wincon0 |= 1;

	/* 2. enable LCD */
	GPEDAT |= (1<<0);

	/* 1. enable LCD's backlight */
	GPFDAT |= (1<<14);
}

static void s3c6410_lcd_hardware_disable(void)
{
	/* 1. disable s3c6410 display controller */
	lcd_regs->vidcon0 &= ~0x3;
	lcd_regs->wincon0 &= ~1;

	/* 2. disable LCD */
	GPEDAT &= ~(1<<0);

	/* 1. disable LCD's backlight */
	GPFDAT &= ~(1<<14);
}
	
int s3c6410_lcd_init(void)
{
	struct clk *lcd_clk = clk_get(NULL, "lcd");
	clk_enable(lcd_clk);
	
	
	/* 2. set fb_info */
	/* 2.1 set variable information */
	memset(&var, 0, sizeof(var));
	var.xres = 480;
	var.yres = 272;
	var.xres_virtual   = 480;
	var.yres_virtual   = 272;
	var.bits_per_pixel = 32;
	
	var.red.offset     = 16;
	var.red.length     = 8;
	
	var.green.offset   = 8;
	var.green.length   = 8;
	
	var.blue.offset    = 0;
	var.blue.length    = 8;

	/* 2.2 set fix information */
	memset(&fix, 0, sizeof(fix));
	// fix.smem_start = //
	fix.smem_len       = 480*272*4;
	fix.type           = FB_TYPE_PACKED_PIXELS;
	fix.visual         = FB_VISUAL_TRUECOLOR;
	fix.line_length    = 480*4;

	/* 2.4 other information */
	//screen_base = /* frame buffer's virtual address */;

	/* alloc frame buffer */
	screen_base = dma_alloc_writecombine(NULL, fix.smem_len, &fix.smem_start, GFP_KERNEL);
	screen_size = fix.smem_len;	
	
	/* 3. hardware setup */
	s3c6410_lcd_hardware_init();
	s3c6410_lcd_hardware_enable();

	major = register_chrdev(0, "mylcd", &mylcd_fops);
	cls = class_create(THIS_MODULE, "mylcd"); /* sysfs */
	device_create(cls, NULL, MKDEV(major, 0), NULL, "fb");
	
	return 0;
}

void s3c6410_lcd_exit(void)
{
	device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major, "mylcd");
	s3c6410_lcd_hardware_disable();
	dma_free_writecombine(NULL, fix.smem_len, screen_base, fix.smem_start);
}

module_init(s3c6410_lcd_init);
module_exit(s3c6410_lcd_exit);
MODULE_LICENSE("GPL");



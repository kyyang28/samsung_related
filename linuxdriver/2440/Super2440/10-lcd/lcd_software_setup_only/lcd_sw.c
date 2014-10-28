

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>

#include <asm/io.h>
#include <asm/div64.h>

#include <asm/mach/map.h>
#include <mach/regs-lcd.h>
#include <mach/regs-gpio.h>
#include <mach/fb.h>


static struct fb_ops s3c2440_lcdfb_ops = {
	.owner		    = THIS_MODULE,
//	.fb_check_var	= s3c2440_lcdfb_check_var,
//	.fb_set_par	    = s3c2440_lcdfb_set_par,
//	.fb_blank	    = s3c2440_lcdfb_blank,
//	.fb_setcolreg	= s3c2440_lcdfb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};


static struct fb_info *s3c_lcd;

static int lcd_init(void)
{
    int ret;
    
    /* 1. Allocating a fb_info structure */
    s3c_lcd = framebuffer_alloc(0, NULL);
    if(!s3c_lcd)
        return -ENOMEM;

    /* 2. Setup this fb_info */
    /* 2.1 Setup the fixed parameters */
    strcpy(s3c_lcd->fix.id, "mylcd");
    s3c_lcd->fix.smem_len       = 272*480*(16/8);
    s3c_lcd->fix.type           = FB_TYPE_PACKED_PIXELS;
    s3c_lcd->fix.visual         = FB_VISUAL_TRUECOLOR;  /* TFT uses TURECOLOR */
    //s3c_lcd->fix.xpanstep       = 0;  // *** framebuffer_alloc() function did the kzmalloc which set the fb_info struct to zero
    //s3c_lcd->fix.ypanstep       = 0;
    //s3c_lcd->fix.ywrapstep      = 0;
    s3c_lcd->fix.line_length    = 272*2; /* one line contains 272 pixels, and 1 pixel is 16 bits = 2 bytes */
        
    
    /* 2.2 Setup the variable parameters */
    s3c_lcd->var.xres               = 272;
    s3c_lcd->var.yres               = 480;
    s3c_lcd->var.xres_virtual       = 272;
    s3c_lcd->var.yres_virtual       = 480;
    s3c_lcd->var.xoffset            = 0;
    s3c_lcd->var.yoffset            = 0;
    s3c_lcd->var.bits_per_pixel     = 16;

    /* R G B : 5 6 5 */
    s3c_lcd->var.red.offset         = 11;   /* start from bit 11 */
    s3c_lcd->var.red.length         = 5;    /* length are 5 bits */

    /* 
     * We don't need to setup the s3c_lcd->var.red.msb_right
     * since s3c_lcd->var.red.msb_right = 0 means MSB is left
     * and framebuffer_alloc() function did the kzmalloc() operation
     * which sets the members of the fb_info struct to zero.
     */

    s3c_lcd->var.green.offset       = 5;    /* start from bit 5 */
    s3c_lcd->var.green.length       = 6;    /* length are 6 bits */
    
    s3c_lcd->var.blue.offset        = 0;    /* start from bit 0 */   
    s3c_lcd->var.blue.length        = 5;    /* length are 5 bits */    

    s3c_lcd->var.activate           = FB_ACTIVATE_NOW;  /* default value */

    /* 2.3 Setup the fb_ops struct */
    s3c_lcd->fbops                  = &s3c2440_lcdfb_ops;

    /* 2.4 Other setup */
    //s3c_lcd->pseudo_palette         = xxx;
    //s3c_lcd->screen_base            = xxx;    /* the virtual address of the framebuffer(display memory) */
    s3c_lcd->screen_size            = 272*480*(16/8);  /* 2 means 2 bytes = 16 bits / 8 */
    
    
    /* 3. Hardware related operations */
    /* 3.1 Setup GPIO for LCD */
    
    
    /* 3.2 According to the S3C2440 manual to setup the LCD controller. e.g. the freq of VCLK, etc. */
    

    /* 3.3 Allocating framebuffer(display memory), then setup the addr of the framebuffer of the LCD controller */
    //s3c_lcd->fix.smem_start = xxx;        /* the physical address of the framebuffer(display memory) */
    
    
    /* 4. Register it using register_framebuffer function */
    ret = register_framebuffer(s3c_lcd); 
    if(ret < 0)
        printk(KERN_ERR "Failed to register framebuffer device: %d\n", ret);

    return 0;
}

static void lcd_exit(void)
{   
    
}

module_init(lcd_init);
module_exit(lcd_exit);
MODULE_LICENSE("GPL");



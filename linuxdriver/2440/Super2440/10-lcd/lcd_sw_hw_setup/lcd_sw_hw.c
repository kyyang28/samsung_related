

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


struct lcd_regs {
    unsigned long LCDCON1;
    unsigned long LCDCON2;
    unsigned long LCDCON3;
    unsigned long LCDCON4;
    unsigned long LCDCON5;
    unsigned long LCDSADDR1;
    unsigned long LCDSADDR2;
    unsigned long LCDSADDR3;
    unsigned long REDLUT;
    unsigned long GREENLUT;
    unsigned long BLUELUT;
    unsigned long RESERVED[9];  /* S3C2440.pdf Chapter 1 LCD Controller p.31 */
    unsigned long DITHMODE;
    unsigned long TPAL;    
    unsigned long LCDINTPND;
    unsigned long LCDSRCPND;    
    unsigned long LCDINTMSK;
    unsigned long TCONSEL;    
};


static struct fb_info *s3c_lcd;

static volatile unsigned long *GPBCON;
static volatile unsigned long *GPBDAT;
static volatile unsigned long *GPCCON;
static volatile unsigned long *GPDCON;
static volatile unsigned long *GPGCON;
static volatile unsigned long *GPGDAT;

static volatile struct lcd_regs *s3c2440_lcd_regs;
static u32 pseudo_pal[16];


static inline unsigned int chan_to_field(unsigned int chan,
					 struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}


static int s3c2440_lcdfb_setcolreg(unsigned regno,
			       unsigned red, unsigned green, unsigned blue,
			       unsigned transp, struct fb_info *info)
{
	unsigned int val;

    if(regno > 16)
        return 1;

    val  = chan_to_field(red,   &info->var.red);
	val |= chan_to_field(green, &info->var.green);
	val |= chan_to_field(blue,  &info->var.blue);

	//((u32 *)(info->pseudo_palette))[regno] = val;
    pseudo_pal[regno] = val;
    return 0;
}


static struct fb_ops s3c2440_lcdfb_ops = {
	.owner		    = THIS_MODULE,
//	.fb_check_var	= s3c2440_lcdfb_check_var,
//	.fb_set_par	    = s3c2440_lcdfb_set_par,
//	.fb_blank	    = s3c2440_lcdfb_blank,
	.fb_setcolreg	= s3c2440_lcdfb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};


static int lcd_init(void)
{
    int ret;
    
    /* 1. Allocating a fb_info structure */
    s3c_lcd = framebuffer_alloc(0, NULL);
    if(!s3c_lcd)
        return -ENOMEM;

    /* 2. Setup this fb_info */    
    /* 2.1 Setup the variable parameters */
    s3c_lcd->var.xres               = 480;
    s3c_lcd->var.yres               = 272;
    s3c_lcd->var.xres_virtual       = 480;
    s3c_lcd->var.yres_virtual       = 272;
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

    s3c_lcd->var.height             = 272;
    s3c_lcd->var.width              = 480;

    s3c_lcd->var.pixclock           = 50000;
	s3c_lcd->var.left_margin        = 3;
	s3c_lcd->var.right_margin       = 3;
	s3c_lcd->var.upper_margin       = 3;
	s3c_lcd->var.lower_margin       = 3;
	s3c_lcd->var.vsync_len          = 11;
	s3c_lcd->var.hsync_len          = 42;

    /* 2.2 Setup the fixed parameters */
    strcpy(s3c_lcd->fix.id, "mylcd");
    s3c_lcd->fix.smem_len       = 480*272*(16/8);
    s3c_lcd->fix.type           = FB_TYPE_PACKED_PIXELS;
    s3c_lcd->fix.visual         = FB_VISUAL_TRUECOLOR;  /* TFT uses TURECOLOR */
    //s3c_lcd->fix.xpanstep       = 0;  // *** framebuffer_alloc() function did the kzmalloc which set the fb_info struct to zero
    //s3c_lcd->fix.ypanstep       = 0;
    //s3c_lcd->fix.ywrapstep      = 0;

    s3c_lcd->fix.smem_start = 0x30000000;
    /* one line contains 480 pixels, and 1 pixel is 16 bits = 2 bytes */
    s3c_lcd->fix.line_length    = (s3c_lcd->var.xres_virtual * s3c_lcd->var.bits_per_pixel) / 8; 
    

    /* 2.3 Setup the fb_ops struct */
    s3c_lcd->fbops                  = &s3c2440_lcdfb_ops;

    /* 2.4 Other setup */
    s3c_lcd->pseudo_palette         = pseudo_pal;
    //s3c_lcd->screen_base            = xxx;    /* the virtual address of the framebuffer(display memory) */
    s3c_lcd->screen_size            = 480*272*(16/8);  /* 2 means 2 bytes = 16 bits / 8 */
    
    
    /* 3. Hardware related operations */
    /* 3.1 Setup GPIO for LCD */
    GPBCON = ioremap(0x56000010, 8);
    GPBDAT = GPBCON + 1;
    GPCCON = ioremap(0x56000020, 4);
    GPDCON = ioremap(0x56000030, 4);
    GPGCON = ioremap(0x56000060, 8);
    GPGDAT = GPGCON + 1;

    *GPCCON = 0xAAAAAAAA;
    *GPDCON = 0xAAAAAAAA;

    *GPBCON &= ~(0x3<<(1*2));  /* Configure the GPB1 to output pin 01 = output */
    *GPBCON |= (0x1<<(1*2));
    *GPBDAT &= ~(0x1<<(1*2));

    *GPGCON &= ~(0x3<<(4*2));
    *GPGCON |= (0x03<<(4*2));   /* GPG4 is set to 11 = LCD_PWRDN */
    //*GPGCON |= (0x01<<(4*2));   /* GPG4 is set to 11 = LCD_PWRDN */
    //*GPGDAT = 1;
    
    /* 3.2 According to the S3C2440 manual to setup the LCD controller. e.g. the freq of VCLK, etc. */
    s3c2440_lcd_regs = ioremap(0x4D000000, sizeof(struct lcd_regs));

    /*  
     *  LCDCON1 0x4D000000 R/W LCD control 1 register
     *  
     *  Referenced by LCD 群创AT043TN24 V.7.pdf p.11
     *
     *  CLKVAL [17:8]:  TFT: VCLK  = HCLK / [(CLKVAL+1) x 2] ( CLKVAL ≥0 )  
     *                     VCLK  = 101.250 MHz / [(CLKVAL+1) x 2]
     *                     11 MHz(110 ns) = 101.250 MHz / [(CLKVAL+1) x 2]
     *                     CLKVAL = 4 = 0x4
     *
     *  PNRMODE [6:5]:  0b11(0x3)   = TFT LCD panel
     *
     *  BPPMODE [4:1]:  0b1100(0xC) = 16 bpp for TFT
     *
     *  ENVID [0]:  
     *              0b0 = Disable the video output and the LCD control signal.
     *              0b1 = Enable the video output and the LCD control signal.
     */
    //s3c2440_lcd_regs->LCDCON1 = (0x4<<8 | 3<<5 | 12<<1);
    s3c2440_lcd_regs->LCDCON1 = (4<<8 | 3<<5 | 12<<1);

    /*  
     *  LCDCON2 0x4D000004 R/W LCD control 2 register  
     *
     *  Referenced by LCD 群创AT043TN24 V.7.pdf p.11 - p.12
     *  
     *  VBPD [31:24] TFT: Vertical back porch is the number of inactive lines at the start of 
     *                    a frame, after vertical synchronization period.
     *                    Tvb = 16 - 1 = 15 = 0xF
     *
     *  LINEVAL [23:14]  TFT/STN: These bits determine the vertical size of LCD panel.
     *                            Tvd = 272 - 1 = 271 = 0x10F
     *
     *  VFPD [13:6]  TFT: Vertical front porch is the number of inactive lines at the end of 
     *                    a frame, before vertical synchronization period.
     *                    Tvb = 16 - 1 = 15 = 0xF
     *
     *  VSPW [5:0]  TFT: Vertical sync pulse width determines the VSYNC pulse's high 
     *                   level width by counting the number of inactive lines. 
     *                   1 - 1 = 0;
     */
    s3c2440_lcd_regs->LCDCON2 = (2<<24) | (271<<14) | (2<<6) | (10<<0); 

    /*
     *  LCDCON3 0x4D000008 R/W LCD control 3 register
     *
     *  HBPD (TFT) [25:19]  TFT: Horizontal back porch is the number of VCLK periods between 
     *                           the falling edge of HSYNC and the start of active data. 
     *
     *  HOZVAL [18:8]  TFT/STN: These bits determine the horizontal size of LCD panel. 
     *
     *  HFPD (TFT) [7:0]  TFT: Horizontal front porch is the number of VCLK periods between 
     *                         the end of active data and the rising edge of HSYNC. 
     */
    s3c2440_lcd_regs->LCDCON3 = (2<<19) | (479<<8) | (2<<0);

    /*
     *  LCDCON4 0x4D00000C R/W LCD control 4 register
     *
     *  HSPW(TFT) [7:0]  TFT: Horizontal sync pulse width determines the HSYNC pulse's 
     *                        high level width by counting the number of the VCLK. 
     */
    s3c2440_lcd_regs->LCDCON4 = (41<<0);
    
    /*
     *  LCDCON5 0x4D000010 R/W LCD control 5 register
     *
     *  FRM565 [11]  TFT: This bit selects the format of 16 bpp output video data. 
     *                    0 = 5:5:5:1 Format    1 = 5:6:5 Format 
     *
     *  BSWP [1]  STN/TFT: Byte swap control bit. 
     *                     0 = Swap Disable(Selected)  1 = Swap Enable 
     *
     *  HWSWP [0]  STN/TFT: Half-Word swap control bit. 
     *                      0 = Swap Disable  1 = Swap Enable(Selected)
     */ 
    s3c2440_lcd_regs->LCDCON5 = (1<<11) | (1<<9) | (1<<8) | (0<<1) | (1<<0);
    
    /* 3.3 Allocating framebuffer(display memory), then setup the addr of the framebuffer of the LCD controller */
    //s3c_lcd->fix.smem_start = xxx;        /* the physical address of the framebuffer(display memory) */
    s3c_lcd->screen_base = dma_alloc_writecombine(NULL, s3c_lcd->fix.smem_len, &s3c_lcd->fix.smem_start, GFP_KERNEL);

    //s3c2440_lcd_regs->LCDSADDR1 = (s3c_lcd->fix.smem_start >> 1) & ~(0x3<<30);
    s3c2440_lcd_regs->LCDSADDR1 = (s3c_lcd->fix.smem_start >> 1);

    //s3c2440_lcd_regs->LCDSADDR2 = ((s3c_lcd->fix.smem_start + (s3c_lcd->fix.smem_len * s3c_lcd->var.yres)) >> 1) & 0x1FFFFF;
    s3c2440_lcd_regs->LCDSADDR2 = ((s3c_lcd->fix.smem_start + (s3c_lcd->fix.smem_len * s3c_lcd->var.yres)) >> 1);

    //s3c2440_lcd_regs->LCDSADDR3 = (480*16/16);
    s3c2440_lcd_regs->LCDSADDR3 = S3C2410_OFFSIZE(0) | S3C2410_PAGEWIDTH((s3c_lcd->fix.line_length / 2) & 0x3ff);

    /* Start(Enable) LCD */
    s3c2440_lcd_regs->LCDCON1 |= 0x1;
    s3c2440_lcd_regs->LCDCON5 |= (1<<3);
    
    /* LCD backlight enabled, output high level */
    *GPBDAT |= (1<<(1*2));

    /* 4. Register it using register_framebuffer function */
    ret = register_framebuffer(s3c_lcd); 
    if(ret < 0)
        printk(KERN_ERR "Failed to register framebuffer device: %d\n", ret);

    return 0;
}

static void lcd_exit(void)
{   
    unregister_framebuffer(s3c_lcd);

    s3c2440_lcd_regs->LCDCON1 &= ~0x1;
    *GPBDAT &= ~(0x1<<(1*2));

    dma_free_writecombine(NULL, s3c_lcd->fix.smem_len, s3c_lcd->screen_base, s3c_lcd->fix.smem_start);

    iounmap(s3c2440_lcd_regs);
    iounmap(GPBCON);
    iounmap(GPCCON);
    iounmap(GPDCON);
    iounmap(GPGCON);

    framebuffer_release(s3c_lcd);
}

module_init(lcd_init);
module_exit(lcd_exit);

MODULE_LICENSE("GPL");



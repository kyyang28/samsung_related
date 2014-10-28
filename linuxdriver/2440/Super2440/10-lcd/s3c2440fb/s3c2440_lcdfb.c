/* linux/drivers/video/s3c2410fb.c
 *	Copyright (c) 2004,2005 Arnaud Patard
 *	Copyright (c) 2004-2008 Ben Dooks
 *
 * S3C2410 LCD Framebuffer Driver
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 * Driver based on skeletonfb.c, sa1100fb.c and others.
*/

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

#define PALETTE_BUFF_CLEAR (0x80000000)	/* entry is clear/invalid */

struct s3c2440fb_info {
	struct device		*dev;
	struct clk		    *clk;

	struct resource		*mem;
	void __iomem		*io;
	//void __iomem		*irq_base;

	struct s3c2410fb_hw	regs;

	unsigned long		clk_rate;
	unsigned int		palette_ready;

	/* keep these registers in case we need to re-write palette */
	u32			palette_buffer[256];
	u32			pseudo_pal[16];
};


/* s3c2410fb_set_lcdaddr
 *
 * initialise lcd controller address pointers
 */
static void s3c2440fb_set_lcdaddr(struct fb_info *info)
{
	unsigned long saddr1, saddr2, saddr3;
	struct s3c2440fb_info *fbi = info->par;
	void __iomem *regs = fbi->io;

	saddr1  = info->fix.smem_start >> 1;
	saddr2  = info->fix.smem_start;
	saddr2 += info->fix.line_length * info->var.yres;
	saddr2 >>= 1;

	saddr3 = S3C2410_OFFSIZE(0) |
		 S3C2410_PAGEWIDTH((info->fix.line_length / 2) & 0x3ff);

	writel(saddr1, regs + S3C2410_LCDSADDR1);
	writel(saddr2, regs + S3C2410_LCDSADDR2);
	writel(saddr3, regs + S3C2410_LCDSADDR3);
}

/* s3c2410fb_calc_pixclk()
 *
 * calculate divisor for clk->pixclk
 */
static unsigned int s3c2410fb_calc_pixclk(struct s3c2440fb_info *fbi,
					  unsigned long pixclk)
{
	unsigned long clk = fbi->clk_rate;
	unsigned long long div;

	/* pixclk is in picoseconds, our clock is in Hz
	 *
	 * Hz -> picoseconds is / 10^-12
	 */

	div = (unsigned long long)clk * pixclk;
	div >>= 12;			/* div / 2^12 */
	do_div(div, 625 * 625UL * 625); /* div / 5^12 */

	return div;
}

/*
 *	s3c2440fb_check_var():
 *	Get the video params out of 'var'. If a value doesn't fit, round it up,
 *	if it's too big, return -EINVAL.
 *
 */
static int s3c2440fb_check_var(struct fb_var_screeninfo *var,
			       struct fb_info *info)
{
	struct s3c2440fb_info *fbi = info->par;
	struct s3c2410fb_mach_info *mach_info = fbi->dev->platform_data;
	struct s3c2410fb_display *display = NULL;
	struct s3c2410fb_display *default_display = mach_info->displays +
						    mach_info->default_display;
	int type = default_display->type;
	unsigned i;

	/* validate x/y resolution */
	/* choose default mode if possible */
	if (var->yres == default_display->yres &&
	    var->xres == default_display->xres &&
	    var->bits_per_pixel == default_display->bpp)
		display = default_display;
	else
		for (i = 0; i < mach_info->num_displays; i++)
			if (type == mach_info->displays[i].type &&
			    var->yres == mach_info->displays[i].yres &&
			    var->xres == mach_info->displays[i].xres &&
			    var->bits_per_pixel == mach_info->displays[i].bpp) {
				display = mach_info->displays + i;
				break;
			}

	if (!display) {
		return -EINVAL;
	}

	/* it is always the size as the display */
	var->xres_virtual = display->xres;
	var->yres_virtual = display->yres;
	var->height = display->height;
	var->width = display->width;

	/* copy lcd settings */
	var->pixclock = display->pixclock;
	var->left_margin = display->left_margin;
	var->right_margin = display->right_margin;
	var->upper_margin = display->upper_margin;
	var->lower_margin = display->lower_margin;
	var->vsync_len = display->vsync_len;
	var->hsync_len = display->hsync_len;

	fbi->regs.lcdcon5 = display->lcdcon5;

    /* set display type */
	fbi->regs.lcdcon1 = display->type;

	var->transp.offset = 0;
	var->transp.length = 0;
	/* set r/g/b positions */
	switch (var->bits_per_pixel) {
	case 1:
	case 2:
	case 4:
		var->red.offset	= 0;
		var->red.length	= var->bits_per_pixel;
		var->green	= var->red;
		var->blue	= var->red;
		break;
	case 8:
		if (display->type != S3C2410_LCDCON1_TFT) {
			/* 8 bpp 332 */
			var->red.length		= 3;
			var->red.offset		= 5;
			var->green.length	= 3;
			var->green.offset	= 2;
			var->blue.length	= 2;
			var->blue.offset	= 0;
		} else {
			var->red.offset		= 0;
			var->red.length		= 8;
			var->green		= var->red;
			var->blue		= var->red;
		}
		break;
	case 12:
		/* 12 bpp 444 */
		var->red.length		= 4;
		var->red.offset		= 8;
		var->green.length	= 4;
		var->green.offset	= 4;
		var->blue.length	= 4;
		var->blue.offset	= 0;
		break;

	default:
	case 16:
		if (display->lcdcon5 & S3C2410_LCDCON5_FRM565) {
			/* 16 bpp, 565 format */
			var->red.offset		= 11;
			var->green.offset	= 5;
			var->blue.offset	= 0;
			var->red.length		= 5;
			var->green.length	= 6;
			var->blue.length	= 5;
		} else {
			/* 16 bpp, 5551 format */
			var->red.offset		= 11;
			var->green.offset	= 6;
			var->blue.offset	= 1;
			var->red.length		= 5;
			var->green.length	= 5;
			var->blue.length	= 5;
		}
		break;
	case 32:
		/* 24 bpp 888 and 8 dummy */
		var->red.length		= 8;
		var->red.offset		= 16;
		var->green.length	= 8;
		var->green.offset	= 8;
		var->blue.length	= 8;
		var->blue.offset	= 0;
		break;
	}
	return 0;
}

/* s3c2410fb_calculate_stn_lcd_regs
 *
 * calculate register values from var settings
 */
static void s3c2440fb_calculate_stn_lcd_regs(const struct fb_info *info,
					     struct s3c2410fb_hw *regs)
{
	const struct s3c2440fb_info *fbi = info->par;
	const struct fb_var_screeninfo *var = &info->var;
	int type = regs->lcdcon1 & ~S3C2410_LCDCON1_TFT;
	int hs = var->xres >> 2;
	unsigned wdly = (var->left_margin >> 4) - 1;
	unsigned wlh = (var->hsync_len >> 4) - 1;

	if (type != S3C2410_LCDCON1_STN4)
		hs >>= 1;

	switch (var->bits_per_pixel) {
	case 1:
		regs->lcdcon1 |= S3C2410_LCDCON1_STN1BPP;
		break;
	case 2:
		regs->lcdcon1 |= S3C2410_LCDCON1_STN2GREY;
		break;
	case 4:
		regs->lcdcon1 |= S3C2410_LCDCON1_STN4GREY;
		break;
	case 8:
		regs->lcdcon1 |= S3C2410_LCDCON1_STN8BPP;
		hs *= 3;
		break;
	case 12:
		regs->lcdcon1 |= S3C2410_LCDCON1_STN12BPP;
		hs *= 3;
		break;

	default:
		/* invalid pixel depth */
		dev_err(fbi->dev, "invalid bpp %d\n",
			var->bits_per_pixel);
	}
	/* update X/Y info */

	regs->lcdcon2 = S3C2410_LCDCON2_LINEVAL(var->yres - 1);

	if (wdly > 3)
		wdly = 3;

	if (wlh > 3)
		wlh = 3;

	regs->lcdcon3 =	S3C2410_LCDCON3_WDLY(wdly) |
			S3C2410_LCDCON3_LINEBLANK(var->right_margin / 8) |
			S3C2410_LCDCON3_HOZVAL(hs - 1);

	regs->lcdcon4 = S3C2410_LCDCON4_WLH(wlh);
}

/* s3c2410fb_calculate_tft_lcd_regs
 *
 * calculate register values from var settings
 */
static void s3c2440fb_calculate_tft_lcd_regs(const struct fb_info *info,
					     struct s3c2410fb_hw *regs)
{
	const struct s3c2440fb_info *fbi = info->par;
	const struct fb_var_screeninfo *var = &info->var;

	switch (var->bits_per_pixel) {
	case 1:
		regs->lcdcon1 |= S3C2410_LCDCON1_TFT1BPP;
		break;
	case 2:
		regs->lcdcon1 |= S3C2410_LCDCON1_TFT2BPP;
		break;
	case 4:
		regs->lcdcon1 |= S3C2410_LCDCON1_TFT4BPP;
		break;
	case 8:
		regs->lcdcon1 |= S3C2410_LCDCON1_TFT8BPP;
		regs->lcdcon5 |= S3C2410_LCDCON5_BSWP |
				 S3C2410_LCDCON5_FRM565;
		regs->lcdcon5 &= ~S3C2410_LCDCON5_HWSWP;
		break;
	case 16:
		regs->lcdcon1 |= S3C2410_LCDCON1_TFT16BPP;
		regs->lcdcon5 &= ~S3C2410_LCDCON5_BSWP;
		regs->lcdcon5 |= S3C2410_LCDCON5_HWSWP;
		break;
	case 32:
		regs->lcdcon1 |= S3C2410_LCDCON1_TFT24BPP;
		regs->lcdcon5 &= ~(S3C2410_LCDCON5_BSWP |
				   S3C2410_LCDCON5_HWSWP |
				   S3C2410_LCDCON5_BPP24BL);
		break;
	default:
		/* invalid pixel depth */
		dev_err(fbi->dev, "invalid bpp %d\n",
			var->bits_per_pixel);
	}
	/* update X/Y info */

	regs->lcdcon2 = S3C2410_LCDCON2_LINEVAL(var->yres - 1) |
			S3C2410_LCDCON2_VBPD(var->upper_margin - 1) |
			S3C2410_LCDCON2_VFPD(var->lower_margin - 1) |
			S3C2410_LCDCON2_VSPW(var->vsync_len - 1);

	regs->lcdcon3 = S3C2410_LCDCON3_HBPD(var->right_margin - 1) |
			S3C2410_LCDCON3_HFPD(var->left_margin - 1) |
			S3C2410_LCDCON3_HOZVAL(var->xres - 1);

	regs->lcdcon4 = S3C2410_LCDCON4_HSPW(var->hsync_len - 1);
}

/* s3c2410fb_activate_var
 *
 * activate (set) the controller from the given framebuffer
 * information
 */
static void s3c2440fb_activate_var(struct fb_info *info)
{
	struct s3c2440fb_info *fbi = info->par;
	void __iomem *regs = fbi->io;
	int type = fbi->regs.lcdcon1 & S3C2410_LCDCON1_TFT;
	struct fb_var_screeninfo *var = &info->var;
	int clkdiv;

	clkdiv = DIV_ROUND_UP(s3c2410fb_calc_pixclk(fbi, var->pixclock), 2);

	if (type == S3C2410_LCDCON1_TFT) {
		s3c2440fb_calculate_tft_lcd_regs(info, &fbi->regs);
		--clkdiv;
		if (clkdiv < 0)
			clkdiv = 0;
	} else {
		s3c2440fb_calculate_stn_lcd_regs(info, &fbi->regs);
		if (clkdiv < 2)
			clkdiv = 2;
	}

	fbi->regs.lcdcon1 |=  S3C2410_LCDCON1_CLKVAL(clkdiv);

	/* write new registers */

	writel(fbi->regs.lcdcon1 & ~S3C2410_LCDCON1_ENVID,
		regs + S3C2410_LCDCON1);
	writel(fbi->regs.lcdcon2, regs + S3C2410_LCDCON2);
	writel(fbi->regs.lcdcon3, regs + S3C2410_LCDCON3);
	writel(fbi->regs.lcdcon4, regs + S3C2410_LCDCON4);
	writel(fbi->regs.lcdcon5, regs + S3C2410_LCDCON5);

	/* set lcd address pointers */
	s3c2440fb_set_lcdaddr(info);

	fbi->regs.lcdcon1 |= S3C2410_LCDCON1_ENVID,
	writel(fbi->regs.lcdcon1, regs + S3C2410_LCDCON1);
}

/*
 *      s3c2440fb_set_par - Alters the hardware state.
 *      @info: frame buffer structure that represents a single frame buffer
 *
 */
static int s3c2440fb_set_par(struct fb_info *info)
{
	struct fb_var_screeninfo *var = &info->var;

	switch (var->bits_per_pixel) {
	case 32:
	case 16:
	case 12:
		info->fix.visual = FB_VISUAL_TRUECOLOR;
		break;
	case 1:
		info->fix.visual = FB_VISUAL_MONO01;
		break;
	default:
		info->fix.visual = FB_VISUAL_PSEUDOCOLOR;
		break;
	}

	info->fix.line_length = (var->xres_virtual * var->bits_per_pixel) / 8;

	/* activate this new configuration */

	s3c2440fb_activate_var(info);
	return 0;
}

static void schedule_palette_update(struct s3c2440fb_info *fbi,
				    unsigned int regno, unsigned int val)
{
	unsigned long flags;

	local_irq_save(flags);

	fbi->palette_buffer[regno] = val;

	if (!fbi->palette_ready) {
		fbi->palette_ready = 1;
	}

	local_irq_restore(flags);
}

/* from pxafb.c */
static inline unsigned int chan_to_field(unsigned int chan,
					 struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}

static int s3c2440fb_setcolreg(unsigned regno,
			       unsigned red, unsigned green, unsigned blue,
			       unsigned transp, struct fb_info *info)
{
	struct s3c2440fb_info *fbi = info->par;
	void __iomem *regs = fbi->io;
	unsigned int val;

	switch (info->fix.visual) {
	case FB_VISUAL_TRUECOLOR:
		/* true-colour, use pseudo-palette */

		if (regno < 16) {
			u32 *pal = info->pseudo_palette;

			val  = chan_to_field(red,   &info->var.red);
			val |= chan_to_field(green, &info->var.green);
			val |= chan_to_field(blue,  &info->var.blue);

			pal[regno] = val;
		}
		break;

	case FB_VISUAL_PSEUDOCOLOR:
		if (regno < 256) {
			/* currently assume RGB 5-6-5 mode */

			val  = (red   >>  0) & 0xf800;
			val |= (green >>  5) & 0x07e0;
			val |= (blue  >> 11) & 0x001f;

			writel(val, regs + S3C2410_TFTPAL(regno));
			schedule_palette_update(fbi, regno, val);
		}

		break;

	default:
		return 1;	/* unknown type */
	}

	return 0;
}

/* s3c2440fb_lcd_enable
 *
 * shutdown the lcd controller
 */
static void s3c2440fb_lcd_enable(struct s3c2440fb_info *fbi, int enable)
{
	unsigned long flags;

	local_irq_save(flags);

	if (enable)
		fbi->regs.lcdcon1 |= S3C2410_LCDCON1_ENVID;
	else
		fbi->regs.lcdcon1 &= ~S3C2410_LCDCON1_ENVID;

	writel(fbi->regs.lcdcon1, fbi->io + S3C2410_LCDCON1);

	local_irq_restore(flags);
}


static struct fb_ops s3c2440fb_ops = {
	.owner		    = THIS_MODULE,
	.fb_check_var	= s3c2440fb_check_var,
	.fb_set_par	    = s3c2440fb_set_par,
	.fb_setcolreg	= s3c2440fb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};

/*
 * s3c2410fb_map_video_memory():
 *	Allocates the DRAM memory for the frame buffer.  This buffer is
 *	remapped into a non-cached, non-buffered, memory region to
 *	allow palette and pixel writes to occur without flushing the
 *	cache.  Once this area is remapped, all virtual memory
 *	access to the video memory should occur at the new region.
 */
static int __devinit s3c2440fb_map_video_memory(struct fb_info *info)
{
	struct s3c2440fb_info *fbi = info->par;
	dma_addr_t map_dma;
	unsigned map_size = PAGE_ALIGN(info->fix.smem_len);

	info->screen_base = dma_alloc_writecombine(fbi->dev, map_size,
						   &map_dma, GFP_KERNEL);

	if (info->screen_base) {
		/* prevent initial garbage on screen */
		memset(info->screen_base, 0x00, map_size);
		info->fix.smem_start = map_dma;
	}

	return info->screen_base ? 0 : -ENOMEM;
}

static inline void s3c2410fb_unmap_video_memory(struct fb_info *info)
{
	struct s3c2440fb_info *fbi = info->par;

	dma_free_writecombine(fbi->dev, PAGE_ALIGN(info->fix.smem_len),
			      info->screen_base, info->fix.smem_start);
}

static inline void modify_gpio(void __iomem *reg,
			       unsigned long set, unsigned long mask)
{
	unsigned long tmp;

	tmp = readl(reg) & ~mask;
	writel(tmp | set, reg);
}

/*
 * s3c2410fb_init_registers - Initialise all LCD-related registers
 */
static int s3c2440fb_init_registers(struct fb_info *info)
{
	struct s3c2440fb_info *fbi = info->par;
	struct s3c2410fb_mach_info *mach_info = fbi->dev->platform_data;
	unsigned long flags;
	void __iomem *regs = fbi->io;
	void __iomem *tpal;
	void __iomem *lpcsel;

	tpal = regs + S3C2410_TPAL;
	lpcsel = regs + S3C2410_LPCSEL;

	/* Initialise LCD with values from haret */

	local_irq_save(flags);

	/* modify the gpio(s) with interrupts set (bjd) */

	modify_gpio(S3C2410_GPCCON, mach_info->gpccon, mach_info->gpccon_mask);
	modify_gpio(S3C2410_GPDCON, mach_info->gpdcon, mach_info->gpdcon_mask);

	local_irq_restore(flags);

	writel(mach_info->lpcsel, lpcsel);

	/* ensure temporary palette disabled */
	writel(0x00, tpal);

	return 0;
}

static int __devinit s3c2440fb_probe(struct platform_device *pdev)
{
    struct s3c2440fb_info *info;
    struct s3c2410fb_display *display;
    struct fb_info *fbinfo;
    struct s3c2410fb_mach_info *mach_info;
    struct resource *res;
    int i, size, lcdcon1;

    mach_info = pdev->dev.platform_data;

    display = mach_info->displays + mach_info->default_display;

    /* Allocating a framebuffer space */
    fbinfo = framebuffer_alloc(sizeof(struct s3c2440fb_info), &pdev->dev);

    platform_set_drvdata(pdev, fbinfo);

    info = fbinfo->par;
    info->dev = &pdev->dev;

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

    size = (res->end - res->start) + 1;
    //info->mem = request_mem_region(res->start, size, pdev->name);

    info->io = ioremap(res->start, size);

    /* Stop the video */
    lcdcon1 = readl(info->io + S3C2410_LCDCON1);
    writel(lcdcon1 & ~S3C2410_LCDCON1_ENVID, info->io + S3C2410_LCDCON1);

    /* Setup fbinfo->fix values */
    fbinfo->fix.type            = FB_TYPE_PACKED_PIXELS;
    fbinfo->fix.type_aux        = 0;
    fbinfo->fix.xpanstep        = 0;
    fbinfo->fix.ypanstep        = 0;
    fbinfo->fix.ywrapstep       = 0;
    fbinfo->fix.accel           = FB_ACCEL_NONE;

    /* Setup fbinfo->var values */
    fbinfo->var.nonstd          = 0;
    fbinfo->var.activate        = FB_ACTIVATE_NOW;
    fbinfo->var.accel_flags     = 0;
    fbinfo->var.vmode           = FB_VMODE_NONINTERLACED;

    fbinfo->fbops               = &s3c2440fb_ops;
    fbinfo->flags               = FBINFO_FLAG_DEFAULT;
    fbinfo->pseudo_palette      = &info->pseudo_pal;

    for (i = 0; i < 256; i++)
        info->palette_buffer[i] = PALETTE_BUFF_CLEAR;

    info->clk = clk_get(NULL, "lcd");

    clk_enable(info->clk);

    msleep(1);

    info->clk_rate = clk_get_rate(info->clk);

    /* find maximum required memory size for display */
    for (i = 0; i < mach_info->num_displays; i++) {
        unsigned long smem_len = mach_info->displays[i].xres;

        smem_len *= mach_info->displays[i].yres;
        smem_len *= mach_info->displays[i].bpp;
        smem_len >>= 3; /* smem_len = xres * yres * bpp / 8 */
        if (fbinfo->fix.smem_len < smem_len)
            fbinfo->fix.smem_len = smem_len;
    }

    /* Initialize(Allocate) video memory */
    s3c2440fb_map_video_memory(fbinfo); /* fbinfo->screen_base */
    
    fbinfo->var.xres = display->xres;
    fbinfo->var.yres = display->yres;
    fbinfo->var.bits_per_pixel = display->bpp;

    /* Setup the GPIO pins to LCD */
    s3c2440fb_init_registers(fbinfo);

    s3c2440fb_check_var(&fbinfo->var, fbinfo);

    /* Register the framebuffer */
    register_framebuffer(fbinfo);
    
    return 0;
}

/*
 *  Cleanup
 */
static int __devexit s3c2440fb_remove(struct platform_device *pdev)
{
	struct fb_info *fbinfo = platform_get_drvdata(pdev);
	struct s3c2440fb_info *info = fbinfo->par;

	unregister_framebuffer(fbinfo);

	s3c2440fb_lcd_enable(info, 0);
	msleep(1);

	s3c2410fb_unmap_video_memory(fbinfo);

	if (info->clk) {
		clk_disable(info->clk);
		clk_put(info->clk);
		info->clk = NULL;
	}

	iounmap(info->io);

	//release_resource(info->mem);
	//kfree(info->mem);

	platform_set_drvdata(pdev, NULL);
	framebuffer_release(fbinfo);

	return 0;
}


static struct platform_driver s3c2440fb_driver = {
	.probe		= s3c2440fb_probe,
	.remove		= s3c2440fb_remove,
	.driver		= {
		.name	= "mylcd",
		.owner	= THIS_MODULE,
	},
};

static int __init s3c2440fb_init(void)
{
	platform_driver_register(&s3c2440fb_driver);
	return 0;
}

static void __exit s3c2440fb_exit(void)
{
	platform_driver_unregister(&s3c2440fb_driver);
}

module_init(s3c2440fb_init);
module_exit(s3c2440fb_exit);

MODULE_LICENSE("GPL");


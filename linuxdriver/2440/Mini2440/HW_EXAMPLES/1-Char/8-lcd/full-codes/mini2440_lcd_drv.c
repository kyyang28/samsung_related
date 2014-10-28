

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>

#include <asm/mach/map.h>
#include <mach/regs-lcd.h>
#include <mach/regs-gpio.h>

#include "mini2440_lcd_drv.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pete&Charles <http://embeddedfansclub.taobao.com>");


static inline unsigned int chan_to_field(unsigned int chan, struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}


static int mini2440_lcdfb_setcolreg(unsigned int regno, unsigned int red,
			     unsigned int green, unsigned int blue,
			     unsigned int transp, struct fb_info *info)
{
    struct mini2440lcd_info *lcd_info = info->par;
	unsigned int val;
	
	if (regno > 16)
		return 1;

	/* 用red,green,blue三原色构造出val */
	val  = chan_to_field(red,	&info->var.red);
	val |= chan_to_field(green, &info->var.green);
	val |= chan_to_field(blue,	&info->var.blue);
	
	lcd_info->pseudo_palette[regno] = val;
	return 0;
}

static struct fb_ops mini2440_lcdfb_ops = {
	.owner		= THIS_MODULE,
	.fb_setcolreg	= mini2440_lcdfb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};


static void mini2440fb_params_init(struct fb_info *fbi)
{
    struct mini2440lcd_info *info = fbi->par;

    /* 设置 */
    /* 设置固定的参数 */
    strcpy(fbi->fix.id, "mini2440_lcd");
    fbi->fix.smem_len       = XRES * YRES * BPP / 8;   /* MINI2440的LCD位宽是24,但是2440里会分配4字节即32位(浪费1字节) */
    fbi->fix.type           = FB_TYPE_PACKED_PIXELS;
    fbi->fix.visual         = FB_VISUAL_TRUECOLOR;     /* TFT */
    fbi->fix.line_length    = XRES_VIRTUAL * BPP / 8;

    
    /* 设置可变的参数 */
    fbi->var.xres           = XRES;
    fbi->var.yres           = YRES;
    fbi->var.xres_virtual   = XRES_VIRTUAL;
    fbi->var.yres_virtual   = YRES_VIRTUAL;
    fbi->var.bits_per_pixel = BPP;

    /* RGB:565 */
    fbi->var.red.offset     = 16;
    fbi->var.red.length     = 8;
    
    fbi->var.green.offset   = 8;
    fbi->var.green.length   = 8;

    fbi->var.blue.offset    = 0;
    fbi->var.blue.length    = 8;

    fbi->var.activate       = FB_ACTIVATE_NOW;
    
    
    /* 设置操作函数 */
    fbi->fbops              = &mini2440_lcdfb_ops;
    
    /* 其他的设置 */
    fbi->pseudo_palette     = info->pseudo_palette; 
    //mini2440_lcd->screen_base  = ;  /* 显存的虚拟地址 */ 
    fbi->screen_size        = XRES * YRES * BPP / 8;
}

static void mini2440lcd_gpio_setup(void)
{
    /* 硬件相关的操作 */
    /* 配置GPIO用于LCD */
    writel(0xAAAAAAAA, S3C2410_GPCCON); /* GPIO管脚用于VD[7:0],LCDVF[2:0],VM,VFRAME,VLINE,VCLK,LEND */
    writel(0xAAAAAAAA, S3C2410_GPDCON); /* GPIO管脚用于VD[23:8] */
    
    writel((readl(S3C2410_GPGCON) | (3<<8)), S3C2410_GPGCON);   /* GPG4用作LCD_PWREN */
}

static void mini2440lcd_enable(struct fb_info *fbi)
{
    struct mini2440lcd_info *info = fbi->par;
    
    /* MINI2440的背光电路也是通过LCD_PWREN来控制的, 不需要单独的背光引脚 */
    /* Enable LCD Controller */
    writel((readl(info->lcd_io + S3C2410_LCDCON1) | (1<<0)), info->lcd_io + S3C2410_LCDCON1);
    /* Enable LCD with LCD_PWREN */
    writel((readl(info->lcd_io + S3C2410_LCDCON5) | (1<<3)), info->lcd_io + S3C2410_LCDCON5);
}


static void mini2440lcd_disable(struct fb_info *fbi)
{
    struct mini2440lcd_info *info = fbi->par;
    
    /* MINI2440的背光电路也是通过LCD_PWREN来控制的, 不需要单独的背光引脚 */
    /* Disable LCD Controller */
    writel((readl(info->lcd_io + S3C2410_LCDCON1) & ~(1<<0)), info->lcd_io + S3C2410_LCDCON1);
    /* Disable LCD with LCD_PWREN */
    writel((readl(info->lcd_io + S3C2410_LCDCON5) & ~(1<<3)), info->lcd_io + S3C2410_LCDCON5);
}


static void mini2440_lcdconx_init(struct mini2440lcd_info *info)
{
    unsigned long tmp;    
    
    /* 
     * MINI2440 LCD 3.5英寸 NEC3.5寸-NL2432HC32.pdf 第16页
     * 
     * LCD手册16,17页和2440手册"Figure 15-6. TFT LCD Timing Example"一对比就知道参数含义了
     */

    /* bit[17:8]: VCLK = HCLK / [(CLKVAL+1) x 2], LCD手册11 (Dclk=5.4MHz~7.2MHz)
     *            5.6MHz = 100MHz / [(CLKVAL+1) x 2]
     *            CLKVAL = 7
     * bit[6:5]: 0b11, TFT LCD
     * bit[4:1]: 0b1100, 16 bpp for TFT
     * bit[0]  : 0 = Disable the video output and the LCD control signal.
     */
    tmp = (0x5<<8) | (0x3<<5) | (0x0c<<1);
    writel(tmp, info->lcd_io + S3C2410_LCDCON1);

    /* 垂直方向的时间参数
     * 根据数据手册
     * bit[31:24]: VBPD, VSYNC之后再过多长时间才能发出第1行数据
     *             LCD手册 tvb=18
     *             VBPD=17
     * bit[23:14]: 多少行, 240, 所以LINEVAL=240-1=239
     * bit[13:6] : VFPD, 发出最后一行数据之后，再过多长时间才发出VSYNC
     *             LCD手册tvf=4, 所以VFPD=4-1=3
     * bit[5:0]  : VSPW, VSYNC信号的脉冲宽度, LCD手册tvp=1, 所以VSPW=1-1=0
     */
     
    /* 使用这些数值, 图像有下移的现象, 应该是数据手册过时了
     * 自己微调一下, 上下移动调VBPD和VFPD
     * 保持(VBPD+VFPD)不变, 减小VBPD图像上移, 取VBPD=11, VFPD=9
     * 多试几次, 我试了10多次
     */
    tmp = ((UPPER_MARGIN-1)<<24) | ((YRES-1)<<14) | \
          ((LOWER_MARGIN-1)<<6) | ((VSYNC_LEN-1)<<0);
    writel(tmp, info->lcd_io + S3C2410_LCDCON2);
    
    /* 水平方向的时间参数
     * bit[25:19]: HBPD, VSYNC之后再过多长时间才能发出第1行数据
     *             LCD手册 thb=38
     *             HBPD=37
     * bit[18:8]: 多少列, 320, 所以HOZVAL=320-1=319
     * bit[7:0] : HFPD, 发出最后一行里最后一个象素数据之后，再过多长时间才发出HSYNC
     *             LCD手册thf>=2, th=408=thp+thb+320+thf, thf=49, HFPD=49-1=48
     */

    /* 使用这些数值, 图像有左移的现象, 应该是数据手册过时了
     * 自己微调一下, 左右移动调HBPD和HFPD
     * 保持(HBPD+HFPD)不变, 增加HBPD图像右移, 取HBPD=69, HFPD=16
     * 多试几次, 我试了10多次
     */
    tmp = ((RIGHT_MARGIN-1)<<19) | ((XRES-1)<<8) | ((LEFT_MARGIN-1)<<0);
    writel(tmp, info->lcd_io + S3C2410_LCDCON3);
    
    /* 水平方向的同步信号
     * bit[7:0] : HSPW, HSYNC信号的脉冲宽度, LCD手册Thp=1, 所以HSPW=1-1=0
     */
    tmp = ((HSYNC_LEN-1)<<0);
    writel(tmp, info->lcd_io + S3C2410_LCDCON4);
    
    /* 信号的极性 
     * bit[11]: 1=565 format, 对于24bpp这个不用设
     * bit[10]: 0 = The video data is fetched at VCLK falling edge
     * bit[9] : 1 = HSYNC信号要反转,即低电平有效 
     * bit[8] : 1 = VSYNC信号要反转,即低电平有效 
     * bit[6] : 0 = VDEN不用反转
     * bit[3] : 0 = PWREN输出0
     *
     * BSWP = 0, HWSWP = 0, BPP24BL = 0 : 当bpp=24时,2440会给每一个象素分配32位即4字节,哪一个字节是不使用的? 看2440手册P412
         * bit[12]: 0, LSB valid, 即最高字节不使用
     * bit[1] : 0 = BSWP
     * bit[0] : 0 = HWSWP
     */
    tmp = (1<<11) | (1<<10) | (1<<9) | (1<<8) | (1<<6) | (0<<12) | \
          (0<<1) | (1<<0);  /* 1<<0 */
    writel(tmp, info->lcd_io + S3C2410_LCDCON5);
}


static inline void mini2440_free_video_memory(struct fb_info *fbi)
{
    dma_free_writecombine(NULL, fbi->fix.smem_len, fbi->screen_base, fbi->fix.smem_start);
}

static int mini2440_alloc_video_memory(struct fb_info *fbi)
{    
    fbi->screen_base = dma_alloc_writecombine(NULL, fbi->fix.smem_len, (dma_addr_t *)&fbi->fix.smem_start, GFP_KERNEL);
    if (fbi->screen_base)
		/* prevent initial garbage on screen */
		memset(fbi->screen_base, 0x00, fbi->fix.smem_len);        

    return fbi->screen_base ? 0 : -ENOMEM;
}

static int mini2440_lcdcontroller_init(struct fb_info *fbi)
{
    struct mini2440lcd_info *info = fbi->par;
    unsigned long tmp;    

    /* Helper function to initialize lcdcon1,2,3,4,5 */
    mini2440_lcdconx_init(info);

    /* 分配显存(framebuffer), 并把地址告诉LCD控制器 */
    if (mini2440_alloc_video_memory(fbi) < 0)
        return -ENOMEM;

    tmp = (fbi->fix.smem_start >> 1) & ~(3<<30);
    writel(tmp, info->lcd_io + S3C2410_LCDSADDR1);

    tmp = ((fbi->fix.smem_start + fbi->fix.smem_len) >> 1) & 0x1fffff;
    writel(tmp, info->lcd_io + S3C2410_LCDSADDR2);

    tmp = (fbi->fix.line_length / 2) & 0x3ff;  /* (320*32/16) 一行的长度(单位: 2字节) */
    writel(tmp, info->lcd_io + S3C2410_LCDSADDR3);

    return 0;
}

static int mini2440_lcd_drv_probe(struct platform_device *pdev)
{
    struct mini2440lcd_info *info;
    struct fb_info *fbinfo;
    struct resource *res;    
    int ret, size;

    /* 1. Allocating a fb_info structure */
    fbinfo = framebuffer_alloc(sizeof(struct mini2440lcd_info), NULL);
    if (!fbinfo)
        return -ENOMEM;
    
	platform_set_drvdata(pdev, fbinfo);

    info = fbinfo->par;

    /* 2. Helper function to initialize framebuffer structure */
    mini2440fb_params_init(fbinfo);

    /* 3. Helper function to setup the LCD related gpio */
    mini2440lcd_gpio_setup();

    /* 4. Get the lcd resources from the platform device side */
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (res == NULL) {
        dev_err(&pdev->dev, "failed to get memory registers\n");
        ret = -ENXIO;
        goto dealloc_fb;
    }

	size = resource_size(res);
	info->lcd_mem = request_mem_region(res->start, size, pdev->name);
	if (info->lcd_mem == NULL) {
		dev_err(&pdev->dev, "failed to get memory region\n");
		ret = -ENOENT;
		goto dealloc_fb;
	}

	info->lcd_io = ioremap(res->start, size);
	if (info->lcd_io == NULL) {
		dev_err(&pdev->dev, "ioremap() of registers failed\n");
		ret = -ENXIO;
		goto release_mem;
	}

    /* 5. Helper function to initialize lcd controller */
    if (mini2440_lcdcontroller_init(fbinfo) < 0) {
        ret = -ENOMEM;
        goto release_video_mem;
    }
    
    /* 6. Helper function to enable the LCD */
    mini2440lcd_enable(fbinfo);
                                
    /* 7. Register */
    if (register_framebuffer(fbinfo) < 0) {
        ret = -EINVAL;
        goto unregister_fb;
    }
    
    return 0;

unregister_fb:    
    mini2440lcd_disable(fbinfo);
    mini2440_free_video_memory(fbinfo);
release_video_mem:
    iounmap(info->lcd_io);
release_mem:
    release_mem_region(res->start, size);
dealloc_fb:
    framebuffer_release(fbinfo);
    return ret;
}

static int mini2440_lcd_drv_remove(struct platform_device *pdev)
{
    struct fb_info *fbinfo = platform_get_drvdata(pdev);
    struct mini2440lcd_info *info = fbinfo->par;
    
    unregister_framebuffer(fbinfo);    
    mini2440lcd_disable(fbinfo);
    mini2440_free_video_memory(fbinfo);
    iounmap(info->lcd_io);
    framebuffer_release(fbinfo);
    return 0;
}

static struct platform_driver mini2440_lcd_driver = {
    .probe              = mini2440_lcd_drv_probe,
    .remove             = __devexit_p(mini2440_lcd_drv_remove),
    .driver             = {
        .owner  = THIS_MODULE,
        .name   = "mini2440-lcd",
    },
};

static int __init mini2440_lcd_drv_init(void)
{
    return platform_driver_register(&mini2440_lcd_driver);
}

static void __exit mini2440_lcd_drv_exit(void)
{
    platform_driver_unregister(&mini2440_lcd_driver);
}

module_init(mini2440_lcd_drv_init);
module_exit(mini2440_lcd_drv_exit);


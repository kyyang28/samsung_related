

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


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pete&Charles <http://embeddedfansclub.taobao.com>");


#define XRES                    240
#define XRES_VIRTUAL            XRES
#define LEFT_MARGIN             10
#define RIGHT_MARGIN            26
#define HSYNC_LEN               6

#define YRES                    320
#define YRES_VIRTUAL            YRES
#define UPPER_MARGIN            2
#define LOWER_MARGIN            16
#define VSYNC_LEN               2

#define BPP                     16


static struct fb_info *mini2440_lcd;
void __iomem        *lcd_io;
struct resource     *lcd_mem;
static u32 pseudo_palette[16];


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
	unsigned int val;
	
	if (regno > 16)
		return 1;

	/* ��red,green,blue��ԭɫ�����val */
	val  = chan_to_field(red,	&info->var.red);
	val |= chan_to_field(green, &info->var.green);
	val |= chan_to_field(blue,	&info->var.blue);
	
	pseudo_palette[regno] = val;
	return 0;
}

static struct fb_ops mini2440_lcdfb_ops = {
	.owner		= THIS_MODULE,
	.fb_setcolreg	= mini2440_lcdfb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};


static void mini2440fb_params_init(void)
{
    /* ���� */
    /* ���ù̶��Ĳ��� */
    strcpy(mini2440_lcd->fix.id, "mylcd");
    mini2440_lcd->fix.smem_len       = XRES * YRES * BPP / 8;   /* MINI2440��LCDλ����24,����2440������4�ֽڼ�32λ(�˷�1�ֽ�) */
    mini2440_lcd->fix.type           = FB_TYPE_PACKED_PIXELS;
    mini2440_lcd->fix.visual         = FB_VISUAL_TRUECOLOR;     /* TFT */
    mini2440_lcd->fix.line_length    = XRES_VIRTUAL * BPP / 8;

    
    /* ���ÿɱ�Ĳ��� */
    mini2440_lcd->var.xres           = XRES;
    mini2440_lcd->var.yres           = YRES;
    mini2440_lcd->var.xres_virtual   = XRES_VIRTUAL;
    mini2440_lcd->var.yres_virtual   = YRES_VIRTUAL;
    mini2440_lcd->var.bits_per_pixel = BPP;

    /* RGB:565 */
    mini2440_lcd->var.red.offset     = 16;
    mini2440_lcd->var.red.length     = 8;
    
    mini2440_lcd->var.green.offset   = 8;
    mini2440_lcd->var.green.length   = 8;

    mini2440_lcd->var.blue.offset    = 0;
    mini2440_lcd->var.blue.length    = 8;

    mini2440_lcd->var.activate       = FB_ACTIVATE_NOW;
    
    
    /* ���ò������� */
    mini2440_lcd->fbops              = &mini2440_lcdfb_ops;
    
    /* ���������� */
    mini2440_lcd->pseudo_palette     = pseudo_palette; 
    //mini2440_lcd->screen_base  = ;  /* �Դ�������ַ */ 
    mini2440_lcd->screen_size        = XRES * YRES * BPP / 8;
}

static void mini2440lcd_gpio_setup(void)
{
    /* Ӳ����صĲ��� */
    /* ����GPIO����LCD */
    writel(0xAAAAAAAA, S3C2410_GPCCON); /* GPIO�ܽ�����VD[7:0],LCDVF[2:0],VM,VFRAME,VLINE,VCLK,LEND */
    writel(0xAAAAAAAA, S3C2410_GPDCON); /* GPIO�ܽ�����VD[23:8] */
    
    writel((readl(S3C2410_GPGCON) | (3<<8)), S3C2410_GPGCON);   /* GPG4����LCD_PWREN */
}

static void mini2440lcd_enable(void)
{
    /* MINI2440�ı����·Ҳ��ͨ��LCD_PWREN�����Ƶ�, ����Ҫ�����ı������� */
    /* ����LCD */
    /* ʹ��LCD������ */
    writel((readl(lcd_io + S3C2410_LCDCON1) | (1<<0)), lcd_io + S3C2410_LCDCON1);
    /* ʹ��LCD����: LCD_PWREN */
    writel((readl(lcd_io + S3C2410_LCDCON5) | (1<<3)), lcd_io + S3C2410_LCDCON5);
}


static void mini2440_lcdconx_init(void)
{
    unsigned long tmp;    

    /* 
     * MINI2440 LCD 3.5Ӣ�� NEC3.5��-NL2432HC32.pdf ��16ҳ
     * 
     * LCD�ֲ�16,17ҳ��2440�ֲ�"Figure 15-6. TFT LCD Timing Example"һ�ԱȾ�֪������������
     */

    /* bit[17:8]: VCLK = HCLK / [(CLKVAL+1) x 2], LCD�ֲ�11 (Dclk=5.4MHz~7.2MHz)
     *            5.6MHz = 100MHz / [(CLKVAL+1) x 2]
     *            CLKVAL = 7
     * bit[6:5]: 0b11, TFT LCD
     * bit[4:1]: 0b1100, 16 bpp for TFT
     * bit[0]  : 0 = Disable the video output and the LCD control signal.
     */
    tmp = (0x5<<8) | (0x3<<5) | (0x0c<<1);
    writel(tmp, lcd_io + S3C2410_LCDCON1);

    /* ��ֱ�����ʱ�����
     * ���������ֲ�
     * bit[31:24]: VBPD, VSYNC֮���ٹ��೤ʱ����ܷ�����1������
     *             LCD�ֲ� tvb=18
     *             VBPD=17
     * bit[23:14]: ������, 240, ����LINEVAL=240-1=239
     * bit[13:6] : VFPD, �������һ������֮���ٹ��೤ʱ��ŷ���VSYNC
     *             LCD�ֲ�tvf=4, ����VFPD=4-1=3
     * bit[5:0]  : VSPW, VSYNC�źŵ�������, LCD�ֲ�tvp=1, ����VSPW=1-1=0
     */
     
    /* ʹ����Щ��ֵ, ͼ�������Ƶ�����, Ӧ���������ֲ��ʱ��
     * �Լ�΢��һ��, �����ƶ���VBPD��VFPD
     * ����(VBPD+VFPD)����, ��СVBPDͼ������, ȡVBPD=11, VFPD=9
     * ���Լ���, ������10���
     */
    tmp = ((UPPER_MARGIN-1)<<24) | ((YRES-1)<<14) | \
          ((LOWER_MARGIN-1)<<6) | ((VSYNC_LEN-1)<<0);
    writel(tmp, lcd_io + S3C2410_LCDCON2);

    /* ˮƽ�����ʱ�����
     * bit[25:19]: HBPD, VSYNC֮���ٹ��೤ʱ����ܷ�����1������
     *             LCD�ֲ� thb=38
     *             HBPD=37
     * bit[18:8]: ������, 320, ����HOZVAL=320-1=319
     * bit[7:0] : HFPD, �������һ�������һ����������֮���ٹ��೤ʱ��ŷ���HSYNC
     *             LCD�ֲ�thf>=2, th=408=thp+thb+320+thf, thf=49, HFPD=49-1=48
     */

    /* ʹ����Щ��ֵ, ͼ�������Ƶ�����, Ӧ���������ֲ��ʱ��
     * �Լ�΢��һ��, �����ƶ���HBPD��HFPD
     * ����(HBPD+HFPD)����, ����HBPDͼ������, ȡHBPD=69, HFPD=16
     * ���Լ���, ������10���
     */
    tmp = ((RIGHT_MARGIN-1)<<19) | ((XRES-1)<<8) | ((LEFT_MARGIN-1)<<0);
    writel(tmp, lcd_io + S3C2410_LCDCON3);

    /* ˮƽ�����ͬ���ź�
     * bit[7:0] : HSPW, HSYNC�źŵ�������, LCD�ֲ�Thp=1, ����HSPW=1-1=0
     */
    tmp = ((HSYNC_LEN-1)<<0);
    writel(tmp, lcd_io + S3C2410_LCDCON4);

    /* �źŵļ��� 
     * bit[11]: 1=565 format, ����24bpp���������
     * bit[10]: 0 = The video data is fetched at VCLK falling edge
     * bit[9] : 1 = HSYNC�ź�Ҫ��ת,���͵�ƽ��Ч 
     * bit[8] : 1 = VSYNC�ź�Ҫ��ת,���͵�ƽ��Ч 
     * bit[6] : 0 = VDEN���÷�ת
     * bit[3] : 0 = PWREN���0
     *
     * BSWP = 0, HWSWP = 0, BPP24BL = 0 : ��bpp=24ʱ,2440���ÿһ�����ط���32λ��4�ֽ�,��һ���ֽ��ǲ�ʹ�õ�? ��2440�ֲ�P412
         * bit[12]: 0, LSB valid, ������ֽڲ�ʹ��
     * bit[1] : 0 = BSWP
     * bit[0] : 0 = HWSWP
     */
    tmp = (1<<11) | (1<<10) | (1<<9) | (1<<8) | (1<<6) | (0<<12) | \
          (0<<1) | (1<<0);  /* 1<<0 */
    writel(tmp, lcd_io + S3C2410_LCDCON5);
}

static void mini2440_lcdcontroller_init(void)
{
    unsigned long tmp;    

    mini2440_lcdconx_init();

    /* �����Դ�(framebuffer), ���ѵ�ַ����LCD������ */
    mini2440_lcd->screen_base = dma_alloc_writecombine(NULL, mini2440_lcd->fix.smem_len, &mini2440_lcd->fix.smem_start, GFP_KERNEL);

    tmp = (mini2440_lcd->fix.smem_start >> 1) & ~(3<<30);
    writel(tmp, lcd_io + S3C2410_LCDSADDR1);

    tmp = ((mini2440_lcd->fix.smem_start + mini2440_lcd->fix.smem_len) >> 1) & 0x1fffff;
    writel(tmp, lcd_io + S3C2410_LCDSADDR2);

    tmp = (mini2440_lcd->fix.line_length / 2) & 0x3ff;  /* (320*32/16) һ�еĳ���(��λ: 2�ֽ�) */
    writel(tmp, lcd_io + S3C2410_LCDSADDR3);
    
    //fbi->fix.smem_start = xxx;  /* �Դ�������ַ */
}

static int mini2440_lcd_drv_probe(struct platform_device *pdev)
{
    struct resource *res;    
    int ret, size;

    /* 1. Allocating a fb_info structure */
    mini2440_lcd = framebuffer_alloc(0, NULL);

    /* 2. Helper function to initialize framebuffer structure */
    mini2440fb_params_init();

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
	lcd_mem = request_mem_region(res->start, size, pdev->name);
	if (lcd_mem == NULL) {
		dev_err(&pdev->dev, "failed to get memory region\n");
		ret = -ENOENT;
		goto dealloc_fb;
	}

	lcd_io = ioremap(res->start, size);
	if (lcd_io == NULL) {
		dev_err(&pdev->dev, "ioremap() of registers failed\n");
		ret = -ENXIO;
		goto release_mem;
	}

    /* 5. Helper function to initialize lcd controller */
    mini2440_lcdcontroller_init();

    /* 6. Helper function to enable the LCD */
    mini2440lcd_enable();
                                
    /* 7. Register */
    if (register_framebuffer(mini2440_lcd) < 0) {
        ret = -EINVAL;
        goto unregister_fb;
    }
    
    return 0;

unregister_fb:    
    writel((readl(lcd_io + S3C2410_LCDCON1) & ~(1<<0)), lcd_io + S3C2410_LCDCON1);
    writel((readl(lcd_io + S3C2410_LCDCON5) & ~(1<<3)), lcd_io + S3C2410_LCDCON5);
    dma_free_writecombine(NULL, mini2440_lcd->fix.smem_len, mini2440_lcd->screen_base, mini2440_lcd->fix.smem_start);
    iounmap(lcd_io);
release_mem:
    release_mem_region(res->start, size);
dealloc_fb:
    framebuffer_release(mini2440_lcd);
    return ret;
}

static int mini2440_lcd_drv_remove(struct platform_device *pdev)
{
    unregister_framebuffer(mini2440_lcd);    
    writel((readl(lcd_io + S3C2410_LCDCON1) & ~(1<<0)), lcd_io + S3C2410_LCDCON1);
    writel((readl(lcd_io + S3C2410_LCDCON5) & ~(1<<3)), lcd_io + S3C2410_LCDCON5);
    dma_free_writecombine(NULL, mini2440_lcd->fix.smem_len, mini2440_lcd->screen_base, mini2440_lcd->fix.smem_start);
    iounmap(lcd_io);
    framebuffer_release(mini2440_lcd);
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



#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/mmc/host.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/mach-types.h>

#include <plat/regs-serial.h>
#include <mach/regs-gpio.h>
#include <mach/regs-lcd.h>
#include <mach/fb.h>


#define LCD_BPP                 16

#define LCD_WIDTH               480
#define LCD_HEIGHT              272
#define LCD_PIXCLOCK            50000

#define LCD_RIGHT_MARGIN        2
#define LCD_LEFT_MARGIN         2
#define LCD_HSYNC_LEN           41

#define LCD_UPPER_MARGIN        2
#define LCD_LOWER_MARGIN        2
#define LCD_VSYNC_LEN           10

static u64 s3c_device_lcd_dmamask = 0xffffffffUL;

static struct s3c2410fb_display smdk2440_lcd_cfg __initdata = {

	.lcdcon5	    = S3C2410_LCDCON5_FRM565 |
			        S3C2410_LCDCON5_INVVLINE |
			        S3C2410_LCDCON5_INVVFRAME |
			        S3C2410_LCDCON5_PWREN |
			        S3C2410_LCDCON5_HWSWP,
    
	.type		    = S3C2410_LCDCON1_TFT,          /* (3<<5) */

	.width		    = LCD_WIDTH,                    /* 480 */
	.height		    = LCD_HEIGHT,                   /* 272 */

	.pixclock	    = LCD_PIXCLOCK,                 /* 50000 */
	.xres		    = LCD_WIDTH,                    /* 480 */
	.yres		    = LCD_HEIGHT,                   /* 272 */
	.bpp		    = LCD_BPP,                      /* 16 */
	.left_margin	= LCD_LEFT_MARGIN + 1,      /* 2 + 1 */
	.right_margin	= LCD_RIGHT_MARGIN + 1,     /* 2 + 1 */
	.hsync_len	    = LCD_HSYNC_LEN + 1,            /* 41 + 1 */
	.upper_margin	= LCD_UPPER_MARGIN + 1,     /* 2 + 1 */
	.lower_margin	= LCD_LOWER_MARGIN + 1,     /* 2 + 1 */
	.vsync_len	    = LCD_VSYNC_LEN + 1,            /* 10 + 1 */
};

static struct s3c2410fb_mach_info smdk2440_fb_info __initdata = {
	.displays	        = &smdk2440_lcd_cfg,
	.num_displays	    = 1,
	.default_display    = 0,

	.gpccon             = 0xaaaaaaaa,
	.gpccon_mask        = 0xffffffff,

	.gpdcon             = 0xaaaaaaaa,
	.gpdcon_mask        = 0xffffffff,

	.lpcsel		        = 0xf82,
};


static struct resource lcd_resource[] = {
	[0] = {
		.start = S3C24XX_PA_LCD,
		.end   = S3C24XX_PA_LCD + S3C24XX_SZ_LCD - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_LCD,
		.end   = IRQ_LCD,
		.flags = IORESOURCE_IRQ,
	}

};


static struct platform_device lcd_device = {
	.name		  = "mylcd",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(lcd_resource),
	.resource	  = lcd_resource,
	.dev              = {
		.dma_mask		= &s3c_device_lcd_dmamask,
		.coherent_dma_mask	= 0xffffffffUL
	}
};


void __init s3c24xx_fb_set_platdata(struct s3c2410fb_mach_info *pd)
{
	struct s3c2410fb_mach_info *npd;

	npd = kmemdup(pd, sizeof(*npd), GFP_KERNEL);
	if (npd) {
		lcd_device.dev.platform_data = npd;
		npd->displays = kmemdup(pd->displays,
			sizeof(struct s3c2410fb_display) * npd->num_displays,
			GFP_KERNEL);
		if (!npd->displays)
			printk(KERN_ERR "no memory for LCD display data\n");
	} else {
		printk(KERN_ERR "no memory for LCD platform data\n");
	}
}


static int __init s3c2440fb_dev_init(void)
{
    s3c24xx_fb_set_platdata(&smdk2440_fb_info);
    platform_device_register(&lcd_device);

    //set GPG4 As Output
	s3c2410_gpio_cfgpin(S3C2410_GPG(4), S3C2410_GPIO_OUTPUT);

    //set backlight on
	s3c2410_gpio_setpin(S3C2410_GPG(4), 1);

    return 0;
}

static void __exit s3c2440fb_dev_exit(void)
{
    platform_device_unregister(&lcd_device);
}

module_init(s3c2440fb_dev_init);
module_exit(s3c2440fb_dev_exit);

MODULE_LICENSE("GPL");


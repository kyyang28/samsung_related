

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>
#include <mach/irqs.h>
#include <mach/map.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pete&Charles <http://embeddedfansclub.taobao.com>");


static struct resource mini2440_wdt_resource[] = {
	[0] = DEFINE_RES_MEM(S3C_PA_WDT, SZ_1K),
	[1] = DEFINE_RES_IRQ(IRQ_WDT),
};


static void mini2440_wdt_dev_release(struct device *dev)
{
}

struct platform_device mini2440_wdt_device = {
	.name		= "mini2440_wdt",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(mini2440_wdt_resource),
	.resource	= mini2440_wdt_resource,	
    .dev                = {
        .release    = mini2440_wdt_dev_release,
    },
};


static int __init mini2440_wdt_dev_init(void)
{
    return platform_device_register(&mini2440_wdt_device);
}

static void __exit mini2440_wdt_dev_exit(void)
{
    platform_device_unregister(&mini2440_wdt_device);
}

module_init(mini2440_wdt_dev_init);
module_exit(mini2440_wdt_dev_exit);


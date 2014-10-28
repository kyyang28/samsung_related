

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <mach/irqs.h>
#include <mach/map.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pete&Charles <http://embeddedfansclub.taobao.com>");


#define MAP_SIZE                    16

static struct resource mini2440_wdt_resource [] = {
    [0] = {
        .start      = S3C_PA_WDT,
        .end        = S3C_PA_WDT + MAP_SIZE - 1,
        .flags      = IORESOURCE_MEM,
    },

    [1] = {
        .start      = IRQ_WDT,
        .end        = IRQ_WDT,
        .flags      = IORESOURCE_IRQ,   
    },
};

static void mini2440_wdt_dev_release(struct device *dev)
{
}

static struct platform_device mini2440_wdt_device = {
    .name               = "mini2440_wdt",
    .id                 = -1,
    .num_resources      = ARRAY_SIZE(mini2440_wdt_resource),
    .resource           = mini2440_wdt_resource,
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


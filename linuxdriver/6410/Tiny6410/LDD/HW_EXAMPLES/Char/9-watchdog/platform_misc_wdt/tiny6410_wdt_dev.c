

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <mach/irqs.h>
#include <mach/map.h>


#define MAP_SIZE                    16

static struct resource tiny6410_wdt_resource [] = {
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

static void tiny6410_wdt_dev_release(struct device *dev)
{
}

static struct platform_device tiny6410_wdt_device = {
    .name               = "tiny6410_wdt",
    .id                 = -1,
    .num_resources      = ARRAY_SIZE(tiny6410_wdt_resource),
    .resource           = tiny6410_wdt_resource,
    .dev                = {
        .release    = tiny6410_wdt_dev_release,
    },
};

static int __init tiny6410_wdt_dev_init(void)
{
    return platform_device_register(&tiny6410_wdt_device);
}

static void __exit tiny6410_wdt_dev_exit(void)
{
    platform_device_unregister(&tiny6410_wdt_device);
}

module_init(tiny6410_wdt_dev_init);
module_exit(tiny6410_wdt_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pete&Charles <http://embeddedfansclub.taobao.com>");



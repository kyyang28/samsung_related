

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>           /* DEFINE_RES_MEM && DEFINE_RES_IRQ */
#include <mach/irqs.h>
#include <mach/map.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pete&Charles <http://embeddedfansclub.taobao.com>");


static void mini2440_ts_dev_release(struct device *dev)
{
}


static struct resource mini2440_ts_resource[] = {
	[0] = DEFINE_RES_MEM(S3C24XX_PA_ADC, S3C24XX_SZ_ADC),
    [1] = DEFINE_RES_IRQ(IRQ_TC),
    [2] = DEFINE_RES_IRQ(IRQ_ADC),
};


struct platform_device mini2440_device_ts = {
	.name		= "mini2440-ts",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(mini2440_ts_resource),
	.resource	= mini2440_ts_resource,
	.dev		= {
		.release    = mini2440_ts_dev_release,
	}
};

static int __init mini2440_ts_dev_init(void)
{
    return platform_device_register(&mini2440_device_ts);
}

static void __exit mini2440_ts_dev_exit(void)
{
    platform_device_unregister(&mini2440_device_ts);
}

module_init(mini2440_ts_dev_init);
module_exit(mini2440_ts_dev_exit);


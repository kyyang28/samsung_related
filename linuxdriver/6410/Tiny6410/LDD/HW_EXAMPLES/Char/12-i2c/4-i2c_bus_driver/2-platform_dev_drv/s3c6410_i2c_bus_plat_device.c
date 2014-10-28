

#include <linux/module.h>  
#include <linux/kernel.h>
#include <linux/init.h> 
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/gpio.h>


#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>
#include <mach/map.h>       /* For IIC0 register */

#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/mach-types.h>


static struct resource s3c6410_i2c_bus_plat_dev_resource[] = {
	[0] = {
		.start = S3C64XX_PA_IIC0,
		.end   = S3C64XX_PA_IIC0 + 16 - 1,
		.flags = IORESOURCE_MEM,
	},
    [1] = {
        .start = IRQ_IIC,
        .flags = IORESOURCE_IRQ,
    },
};

static void	s3c6410_i2c_bus_plat_dev_release(struct device *dev)
{
}

static struct platform_device s3c6410_i2c_bus_plat_dev = {
	.name		= "i2c_s3c6410",
	.id		    = -1,
	.num_resources	= ARRAY_SIZE(s3c6410_i2c_bus_plat_dev_resource),
	.resource	= s3c6410_i2c_bus_plat_dev_resource,
	.dev        = {
        .release = s3c6410_i2c_bus_plat_dev_release,
	},
};


static int __init s3c6410_i2c_bus_plat_device_init(void)
{
    platform_device_register(&s3c6410_i2c_bus_plat_dev);
    return 0;
}

static void __exit s3c6410_i2c_bus_plat_device_exit(void)
{
    platform_device_unregister(&s3c6410_i2c_bus_plat_dev);
}

module_init(s3c6410_i2c_bus_plat_device_init);
module_exit(s3c6410_i2c_bus_plat_device_exit);

MODULE_LICENSE("GPL");


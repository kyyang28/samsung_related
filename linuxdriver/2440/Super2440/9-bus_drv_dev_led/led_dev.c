
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

#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/mach-types.h>


static struct resource led_resource[] = {
	[0] = {
		.start = 0x56000050,
		.end   = 0x56000050 + 8 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = 6,
		.end   = 6,
		.flags = IORESOURCE_IRQ,
	}
};


static void led_release(struct device *dev)
{
    printk("led_dev.c: led_release is invoking\n");
}

struct platform_device led_dev = {
	.name		    = "myled",
	.id		        = -1,
	.num_resources	= ARRAY_SIZE(led_resource),
	.resource	    = led_resource,
	.dev            = {
        .release = led_release,
	}
};


static int led_dev_init(void)
{
    platform_device_register(&led_dev);
    return 0;
}


static void led_dev_exit(void)
{
    platform_device_unregister(&led_dev);
}

module_init(led_dev_init);
module_exit(led_dev_exit);

MODULE_LICENSE("GPL");


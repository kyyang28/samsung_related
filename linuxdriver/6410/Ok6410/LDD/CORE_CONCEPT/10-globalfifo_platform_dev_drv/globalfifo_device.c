
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>      /* platform_device structure */


static void globalfifo_release(struct device *dev)
{
}


static struct platform_device globalfifo_device = {
    .name       = "globalfifo",
    .id         = -1,
    .dev        = {
        .release  = globalfifo_release,
    }
};

static int __init globalfifo_platform_device_init(void)
{
    return platform_device_register(&globalfifo_device);
}

static void __exit globalfifo_platform_device_exit(void)
{
    platform_device_unregister(&globalfifo_device);
}

module_init(globalfifo_platform_device_init);
module_exit(globalfifo_platform_device_exit);

MODULE_LICENSE("GPL");


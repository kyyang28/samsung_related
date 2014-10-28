
#include <linux/kernel.h>
#include <linux/module.h>

static int __init hello_init(void)
{
    printk("Hello_init is invoked!\n");
    return 0;
}

static void __exit hello_exit(void)
{
    printk("hello_exit is invoked!\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");



#include <linux/kernel.h>               /* printk */
#include <linux/module.h>
#include <linux/init.h>                 /* __init / __exit */

static int __init tiny210_hello_init(void)
{
    printk("Invoking tiny210_hello_init!\n");
    return 0;
}

static void __exit tiny210_hello_exit(void)
{
    printk("Invoking tiny210_hello_exit!\n");
}

module_init(tiny210_hello_init);
module_exit(tiny210_hello_exit);

MODULE_LICENSE("GPL");


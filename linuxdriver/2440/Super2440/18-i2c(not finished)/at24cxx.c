

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>


static struct i2c_client_address_data addr_data = {
	.normal_i2c	= normal_addr,  /* 要发出S信号和设备地址并得到ACK信号,才能确定存在这个设备 */
	.probe		= ignore,
	.ignore		= ignore,
	//.forces     = forces, /* 强制认为存在这个设备 */
};

static int at24cxx_attach_adapter(struct i2c_adapter *adap)
{
    return i2c_probe(adap, &addr_data, at24cxx_detect);
}

static int at24cxx_detach_adapter(struct i2c_adapter *adap)
{
    printk("at24cxx_detach_adapter\n");
    return 0;
}


/* 1. Allocate a i2c_driver struct */
/* 2. Configure at24cxx_i2c_driver struct */
static struct i2c_driver at24cxx_driver = {
	.driver = {
		.name	= "at24cxx",
	},
	.attach_adapter	= at24cxx_attach_adapter,
	.detach_adapter	= at24cxx_detach_adapter,
};


static int __init at24cxx_init(void)
{
    i2c_add_driver(&at24cxx_driver);
    return 0;
}

static void __exit at24cxx_exit(void)
{
    i2c_del_driver(&at24cxx_driver);
}

module_init(at24cxx_init);
module_exit(at24cxx_exit);

MODULE_LICENSE("Dual BSD/GPL");



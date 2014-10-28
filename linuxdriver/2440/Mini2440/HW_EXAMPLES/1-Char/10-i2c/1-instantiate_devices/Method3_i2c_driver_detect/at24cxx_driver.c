

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/slab.h>


static const struct i2c_device_id at24cxx_id_table[] = {
	{ "at24c08", 0 },
};

static const unsigned short at24cxx_i2c[] = {
	0x60, 0x70, 0x50, I2C_CLIENT_END 
};

static int __devinit at24cxx_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
    printk("[at24cxx_driver.c]In function %s, at line %d\n", __FUNCTION__, __LINE__);
    return 0;
}

static int __devexit at24cxx_remove(struct i2c_client *client)
{
    printk("[at24cxx_driver.c]In function %s, at line %d\n", __FUNCTION__, __LINE__);
    return 0;
}


static int at24cxx_detect(struct i2c_client *new_client,
		       struct i2c_board_info *info)
{
    printk("[at24cxx_detect]addr = 0x%x\n", new_client->addr);
    if (new_client->addr == 0x50) {
        strlcpy(info->type, "at24c08", I2C_NAME_SIZE);        
        return 0;
    }else
        return -ENODEV;
}

static struct i2c_driver at24cxx_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver = {
		.name	= "at24cxx",
        .owner  = THIS_MODULE,
	},
	.probe		= at24cxx_probe,
	.remove		= __devexit_p(at24cxx_remove),
	.id_table	= at24cxx_id_table,
	.detect		= at24cxx_detect,
	.address_list	= at24cxx_i2c,
};


static int __init at24cxx_driver_init(void)
{
    /* Register i2c_driver structure */
	int ret;
	ret = i2c_add_driver(&at24cxx_driver);
	if (ret != 0)
		pr_err("Failed to register at24cxx I2C driver: %d\n", ret);
	return ret;
}

static void __exit at24cxx_driver_exit(void)
{
    i2c_del_driver(&at24cxx_driver);
}

module_init(at24cxx_driver_init);
module_exit(at24cxx_driver_exit);

MODULE_LICENSE("GPL");


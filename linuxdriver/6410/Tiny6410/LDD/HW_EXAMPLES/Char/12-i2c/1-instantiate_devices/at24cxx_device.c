

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/slab.h>


static struct i2c_client *at24cxx_client;

static const unsigned short at24cxx_address_list[] = { 
    0x60, 0x70, 0x50, I2C_CLIENT_END 
};

static int __init at24cxx_device_init(void)
{
    int ret = 0;
    struct i2c_adapter *i2c_adap;
    struct i2c_board_info at24cxx_info;
    
    /* Get the i2c_adapter */
    i2c_adap = i2c_get_adapter(0);

    memset(&at24cxx_info, 0, sizeof(struct i2c_board_info));
	strlcpy(at24cxx_info.type, "at24c08", I2C_NAME_SIZE);
    
    /* Using i2c_new_device explicitly */
    at24cxx_client = i2c_new_probed_device(i2c_adap, &at24cxx_info, at24cxx_address_list, NULL);

    /* Put the i2c_adapter */
    i2c_put_adapter(i2c_adap);

    if (!at24cxx_client) {
        ret = -ENODEV;
    }

    return ret;
}

static void __exit at24cxx_device_exit(void)
{
    i2c_unregister_device(at24cxx_client);
}

module_init(at24cxx_device_init);
module_exit(at24cxx_device_exit);

MODULE_LICENSE("GPL");


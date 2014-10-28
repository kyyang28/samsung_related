

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/gfp.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>


static struct net_device *tiny6410_vnet;


static const struct net_device_ops vnet_netdev_ops = {

};

static int __init tiny6410_vnet_init(void)
{
	/*  1. Allocating a net_device structure(tiny6410_vnet) */
	tiny6410_vnet = alloc_netdev(0, "lynet%d", ether_setup);;  /* alloc_etherdev */

	/*  2. Setup tiny6410_vnet */
    /* The net operations of the net_device structure */
    tiny6410_vnet->netdev_ops = &vnet_netdev_ops;

	/* 
	 *  3. Register tiny6410_vnet
	 *  DONOT use register_netdevice(tiny6410_vnet) since this function does not do 
	 *  rtnl_lock().
	 */
	register_netdev(tiny6410_vnet);
    
    return 0;
}

static void __exit tiny6410_vnet_exit(void)
{
	unregister_netdev(tiny6410_vnet);
	free_netdev(tiny6410_vnet);
}

module_init(tiny6410_vnet_init);
module_exit(tiny6410_vnet_exit);

MODULE_AUTHOR("Pete&Charles <http://embeddedfansclub.taobao.com>");
MODULE_LICENSE("GPL");


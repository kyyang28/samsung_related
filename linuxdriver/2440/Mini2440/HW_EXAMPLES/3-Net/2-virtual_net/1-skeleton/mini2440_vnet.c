

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


MODULE_AUTHOR("Pete&Charles <http://embeddedfansclub.taobao.com>");
MODULE_LICENSE("GPL");


static struct net_device *mini2440_vnet;


static const struct net_device_ops vnet_netdev_ops = {

};

static int __init mini2440_vnet_init(void)
{
	/*  1. Allocating a net_device structure(mini2440_vnet) */
	mini2440_vnet = alloc_netdev(0, "efc%d", ether_setup);;  /* alloc_etherdev */

	/*  2. Setup mini2440_vnet */
    /*  The net operations of the net_device structure */
    /* 
     *  MUST INIT the net_device_ops, otherwise, 
     *  the system will report NULL pointer exception
     */
    mini2440_vnet->netdev_ops = &vnet_netdev_ops;   

	/* 
	 *  3. Register mini2440_vnet
	 *  DONOT use register_netdevice(mini2440_vnet) since this function does not do 
	 *  rtnl_lock().
	 */
	register_netdev(mini2440_vnet);
    
    return 0;
}

static void __exit mini2440_vnet_exit(void)
{
	unregister_netdev(mini2440_vnet);
	free_netdev(mini2440_vnet);
}

module_init(mini2440_vnet_init);
module_exit(mini2440_vnet_exit);


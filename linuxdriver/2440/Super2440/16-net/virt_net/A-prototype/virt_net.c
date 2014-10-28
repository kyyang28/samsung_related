
/*
 *  Referenced by linux-2.6.38\drivers\net\cs89x0.c
 */

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


static struct net_device *virt_net_device;  /* vnet */

static int virt_net_init(void)
{
    /* Allocate a net_device struct */
    virt_net_device = alloc_netdev(0, "vnet%d", ether_setup);
    
    /* Configure */
    
    
    /* Register */
    //register_netdevice(virt_net_device);
    register_netdev(virt_net_device);
    
    /* Hardware related operations */
    
    
    return 0;
}


static void virt_net_exit(void)
{
    unregister_netdev(virt_net_device);
    free_netdev(virt_net_device);
}

module_init(virt_net_init);
module_exit(virt_net_exit);

MODULE_AUTHOR("Charles Yang <charlesyang28@gmail.com>");
MODULE_LICENSE("Dual BSD/GPL");



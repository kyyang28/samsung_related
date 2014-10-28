

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
#include <linux/ip.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>


static struct net_device *tiny6410_vnet;


static void construct_rxpack(struct sk_buff *skb, struct net_device *dev)
{ 
	unsigned char *type;
	struct iphdr *ih;
	__be32 *saddr, *daddr, tmp;
	unsigned char	tmp_dev_addr[ETH_ALEN];
	struct ethhdr *ethhdr;
	
	struct sk_buff *rx_skb;
		
    /* 
     *  Read and save the data from hardware 
     *  Switch the source/destination of the MAC address 
     */
	ethhdr = (struct ethhdr *)skb->data;
	memcpy(tmp_dev_addr, ethhdr->h_dest, ETH_ALEN);
	memcpy(ethhdr->h_dest, ethhdr->h_source, ETH_ALEN);
	memcpy(ethhdr->h_source, tmp_dev_addr, ETH_ALEN);

    /* 
     *  Switch the source/destination of the IP address 
     */
	ih = (struct iphdr *)(skb->data + sizeof(struct ethhdr));
	saddr = &ih->saddr;
	daddr = &ih->daddr;

	tmp = *saddr;
	*saddr = *daddr;
	*daddr = tmp;
	
	//((u8 *)saddr)[2] ^= 1; /* change the third octet (class C) */
	//((u8 *)daddr)[2] ^= 1;
	type = skb->data + sizeof(struct ethhdr) + sizeof(struct iphdr);
	//printk("tx package type = %02x\n", *type);

    /* 
     *  Modify the type, 0x8 means ping
     */
	*type = 0; /* 0 means reply */
	
	ih->check = 0;		   /* and rebuild the checksum (ip needs it) */
	ih->check = ip_fast_csum((unsigned char *)ih,ih->ihl);
	
    /* Construct a sk_buff structure */
    rx_skb = dev_alloc_skb(skb->len + 2);
	skb_reserve(rx_skb, 2); /* align IP on 16B boundary */	
	memcpy(skb_put(rx_skb, skb->len), skb->data, skb->len);

	/* Write metadata, and then pass to the receive level */
	rx_skb->dev = dev;
	rx_skb->protocol = eth_type_trans(rx_skb, dev);
	rx_skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */
	dev->stats.rx_packets++;
	dev->stats.rx_bytes += skb->len;

	/* Submit sk_buff */
	netif_rx(rx_skb);
}


static netdev_tx_t tiny6410_vnet_sendpack(struct sk_buff *skb,
                       struct net_device *dev)
{
    static int counter = 0;
    printk("[DRIVER]counter = %d\n", ++counter);

    /* Stop the net queue */
    netif_stop_queue(dev);

    /* Construct a counterfeit packet */
    construct_rxpack(skb, dev);

    /* Free the skb packet */
    dev_kfree_skb(skb);

    /* Wakeup the net queue */
    netif_wake_queue(dev);
    
    /* Updating the tx stats */
    dev->stats.tx_packets++;
    dev->stats.tx_bytes += skb->len;

    return 0;
}


static const struct net_device_ops vnet_netdev_ops = {
    .ndo_start_xmit         = tiny6410_vnet_sendpack,
};

static int __init tiny6410_vnet_init(void)
{
	/*  1. Allocating a net_device structure(tiny6410_vnet) */
	tiny6410_vnet = alloc_netdev(0, "lyeth%d", ether_setup);;  /* alloc_etherdev */

	/*  2. Setup tiny6410_vnet */
    /* The net operations of the net_device structure */
    tiny6410_vnet->netdev_ops = &vnet_netdev_ops;

    /*  3. Setup the MAC address */
    tiny6410_vnet->dev_addr[0] = 0x1A;
    tiny6410_vnet->dev_addr[1] = 0x2B;
    tiny6410_vnet->dev_addr[2] = 0x3C;
    tiny6410_vnet->dev_addr[3] = 0x4D;
    tiny6410_vnet->dev_addr[4] = 0x5E;
    tiny6410_vnet->dev_addr[5] = 0x6F;

    /* Need to setup the following 2 members in order to ping successfully */
	tiny6410_vnet->flags           |= IFF_NOARP;
	tiny6410_vnet->features        |= NETIF_F_NO_CSUM;	


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


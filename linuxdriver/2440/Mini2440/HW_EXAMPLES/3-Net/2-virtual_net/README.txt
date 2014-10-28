

1. 	网卡驱动程序的入口函数一定要初始化，否则insmod的时候会出现NULL Pointer Exception
		vnet_dev->netdev_ops = &vnet_netdev_ops;

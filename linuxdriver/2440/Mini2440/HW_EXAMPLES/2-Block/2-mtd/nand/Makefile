KERNEL_DIR = /home/kyyang/yan/kernel/linux-3.5.4_mini2440

all:
	make -C $(KERNEL_DIR) M=`pwd` modules

clean:
	make -C $(KERNEL_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m	+= mini2440_nand_drv.o 
obj-m	+= mini2440_nand_dev.o 


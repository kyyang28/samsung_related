KERNEL_DIR = /home/kyyang/yan/kernel/linux-2.6.32.2_mini2440 

all:
	make -C $(KERNEL_DIR) M=`pwd` modules

clean:
	make -C $(KERNEL_DIR) M=`pwd` modules clean
	rm -rf modules.order *.o *.ko

obj-m	+= second_drv.o


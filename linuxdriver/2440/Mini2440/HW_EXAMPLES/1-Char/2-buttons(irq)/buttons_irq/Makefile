KERNEL_DIR = /home/kyyang/yan/kernel/linux-2.6.32.2

all:
	make -C $(KERNEL_DIR) M=`pwd` modules

clean:
	make -C $(KERNEL_DIR) M=`pwd` clean

obj-m += buttonsIRQ_leds_drv.o 


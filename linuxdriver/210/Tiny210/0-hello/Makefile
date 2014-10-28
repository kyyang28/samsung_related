KERNEL_DIR = /home/kyyang/yan/kernel/linux-2.6.35.7

all:
	make -C $(KERNEL_DIR) M=`pwd` modules

clean:
	make -C $(KERNEL_DIR) M=`pwd` clean

obj-m += tiny210_hello.o


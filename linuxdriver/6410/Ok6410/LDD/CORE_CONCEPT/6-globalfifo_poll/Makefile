KERNEL_DIR = /home/kyyang/yan/kernel/linux-3.0.1_ok6410

all:
	make -C $(KERNEL_DIR) M=`pwd` modules

clean:
	make -C $(KERNEL_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m	+= globalfifo.o


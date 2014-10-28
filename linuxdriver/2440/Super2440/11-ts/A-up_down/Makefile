KERNEL_DIR = /home/kyyang/yan/kernel/linux-2.6.38_Super

all:
	make -C $(KERNEL_DIR) M=`pwd` modules

clean:
	make -C $(KERNEL_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m	+= s3c_ts.o 


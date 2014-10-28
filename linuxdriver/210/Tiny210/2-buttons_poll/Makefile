KERNEL_DIR = /home/kyyang/yan/kernel/linux-2.6.35.7

all : modules test

modules:
	make -C $(KERNEL_DIR) M=`pwd` modules

test:
	arm-linux-gcc -Wall tiny210_buttonsPoll_test.c -o tiny210_buttonsPoll_test

clean_modules:
	make -C $(KERNEL_DIR) M=`pwd` clean

clean_test:
	rm -f tiny210_buttonsPoll_test

clean : clean_modules clean_test

install:
	cp *.ko /home/kyyang/yan/nfs/images
	cp tiny210_buttonsPoll_test /home/kyyang/yan/nfs/images

obj-m += tiny210_buttonsPoll_drv.o 


KERNEL_DIR = /home/kyyang/yan/kernel/linux-2.6.35.7

all : modules

modules:
	make -C $(KERNEL_DIR) M=`pwd` modules

clean_modules:
	make -C $(KERNEL_DIR) M=`pwd` clean

clean_test:
	rm -f tiny210_leds_test

clean : clean_modules clean_test

test:
	arm-linux-gcc -Wall tiny210_leds_test.c -o tiny210_leds_test

install:
	cp *.ko /home/kyyang/yan/nfs/images
	cp tiny210_leds_test /home/kyyang/yan/nfs/images

obj-m += tiny210_leds_drv.o 


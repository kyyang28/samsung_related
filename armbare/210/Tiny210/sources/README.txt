SD Card burning method£º

Step 1: Check the SD device
		ls /dev/sd*

		Output something like this:
		/dev/sda  /dev/sda1  /dev/sda2  /dev/sda5  /dev/sdb  /dev/sdb1

Step 2:	Burning the bin file into SD card
		sudo dd iflag=dsync oflag=dsync if=xxx.bin of=/dev/sdb seek=1 
		(where xxx.bin is the bin file including the header info)

Nandflash burning method£º

Step 1:	Booting u-boot-tiny210v2 from SD Card

Step 2: Using nand cmd to burn bare program bin file
				
				loady
				
				nand erase 0 100000
				
				nand down
				
Step 3:	Booting from nandflash


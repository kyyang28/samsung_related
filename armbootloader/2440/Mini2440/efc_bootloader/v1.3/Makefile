
CC      = arm-linux-gcc
LD      = arm-linux-ld
AR      = arm-linux-ar
OBJCOPY = arm-linux-objcopy
OBJDUMP = arm-linux-objdump

INCLUDEDIR  := $(shell pwd)/include
CFLAGS 		:= -Wall -O2
CPPFLAGS   	:= -nostdinc -fno-builtin -I$(INCLUDEDIR)
LIBPATH     := -L/home/kyyang/yan/cross_compiler/usr/local/arm/4.3.2/lib/gcc/arm-none-linux-gnueabi/4.3.2 
LIBPATH     += -lgcc

export AR CC LD OBJCOPY OBJDUMP INCLUDEDIR CFLAGS CPPFLAGS

#OBJS := start.o lowlevel_init.o nand.o boot_init.o serial.o
OBJS := nand.o serial.o
OBJS += bootm.o
OBJS += kermit.o

OBJS += lib/libc.a
OBJS += common/cmd.a
OBJS += system/system.a
OBJS += init/init.a
#OBJS += drivers/drivers.a

TARGET := efc_boot_v1.3

$(TARGET).bin: $(OBJS)
	${LD} -Tefc_boot.lds -o $(TARGET).elf $^ ${LIBPATH}
	${OBJCOPY} -O binary -S $(TARGET).elf $@
	${OBJDUMP} -D -m arm $(TARGET).elf > $(TARGET).dis

.PHONY: init/init.a lib/libc.a common/cmd.a system/system.a #drivers/drivers.a
init/init.a:
	cd init; make; cd ..

lib/libc.a:
	cd lib; make; cd ..

common/cmd.a:
	cd common; make; cd ..

system/system.a:
	cd system; make; cd ..

#drivers/drivers.a:
	#cd drivers; make; cd ..

%.o:%.c
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%.o:%.S
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f *.bin *.elf *.dis *.o 
	rm -f init/*.o init/*.a lib/*.o lib/*.a common/*.o common/*.a system/*.o system/*.a #drivers/*.o drivers/*.a
	

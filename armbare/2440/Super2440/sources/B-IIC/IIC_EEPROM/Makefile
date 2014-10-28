
AR      = arm-linux-ar
CC      = arm-linux-gcc
LD      = arm-linux-ld
OBJCOPY = arm-linux-objcopy
OBJDUMP = arm-linux-objdump

INCLUDEDIR := $(shell pwd)/include
CFLAGS := -Wall -DLARGER_NAND_PAGE
CPPFLAGS := -nostdinc -I$(INCLUDEDIR)

export AR CC LD OBJCOPY OBJDUMP INCLUDEDIR CFLAGS CPPFLAGS

OBJS := start.o init.o lowlevel_init.o nand.o uart.o main.o

OBJS += IIC.o 
OBJS += interrupt.o IIC_interrupt.o
OBJS += eeprom.o

OBJS += lib/libc.a

TARGET := IIC_EEPROM

$(TARGET).bin : $(OBJS)
	$(LD) -TIIC.lds -o $(TARGET).elf $^
	$(OBJCOPY) -O binary -S $(TARGET).elf $@
	$(OBJDUMP) -D -m arm $(TARGET).elf > $(TARGET).dis
	ls -l $@

.PHONY: lib/libc.a
lib/libc.a:
	cd lib; make; cd ..

%.o : %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

%.o : %.S
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f $(TARGET).bin $(TARGET).elf $(TARGET).dis *.o lib/*.o lib/*.a


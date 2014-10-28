
CROSS_COMPILE = arm-linux-

AR = $(CROSS_COMPILE)ar
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

INCLUDEDIR := $(shell pwd)/include
CFLAGS := -Wall -Os -fno-builtin-printf -nostdinc -I$(INCLUDEDIR) -DENABLE_UART_FIFO -DBOOT_FROM_NAND
LDFLAGS := -T uart.lds
TARGET = uart_stdio_ddr

export AR CC LD OBJCOPY OBJDUMP INCLUDEDIR CFLAGS

COBJS := start.o clock.o ddr.o uart.o myuart.o
COBJS += lib/libc.a

$(TARGET).bin : $(COBJS)
	$(LD) $(LDFLAGS) $^ -o $(TARGET).elf
	$(OBJCOPY) -O binary $(TARGET).elf $@
	$(OBJDUMP) -D $(TARGET).elf > $(TARGET).dis

.PHONY : lib/libc.a
lib/libc.a : 
	cd lib; make; cd ..

%.o : %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean :
	make clean -C lib
	rm -f *.o $(TARGET).bin $(TARGET).elf $(TARGET).dis


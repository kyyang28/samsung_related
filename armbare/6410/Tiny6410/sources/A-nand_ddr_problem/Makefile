
CROSS_COMPILE = arm-linux-

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

COBJS = start.o led.o clock.o ddr.o nand.o mynand.o
CFLAGS = -Wall
CFLAGS += -DBOOT_FROM_NAND
LDFLAGS = -T nand.lds
TARGET = nand_relocate

$(TARGET).bin : $(COBJS)
	$(LD) $(LDFLAGS) $^ -o $(TARGET).elf
	$(OBJCOPY) -O binary $(TARGET).elf $@
	$(OBJDUMP) -D $(TARGET).elf > $(TARGET).dis

%.o : %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean :
	rm -f *.o $(TARGET).bin $(TARGET).elf $(TARGET).dis


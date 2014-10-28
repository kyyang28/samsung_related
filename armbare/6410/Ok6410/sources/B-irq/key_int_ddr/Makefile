
CROSS_COMPILE = arm-linux-

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

COBJS = start.o
COBJS += clock.o ddr.o
COBJS += irq.o led.o main.o
CFLAGS = -Wall -DBOOT_FROM_NAND -DBOOT_ON_DDR 
LDFLAGS = -T keyint.lds
TARGET = keyint_DDR

$(TARGET).bin : $(COBJS)
	$(LD) $(LDFLAGS) $^ -o $(TARGET).elf
	$(OBJCOPY) -O binary $(TARGET).elf $@
	$(OBJDUMP) -D $(TARGET).elf > $(TARGET).dis

%.o : %.S
	$(CC) $(CFLAGS) $< -c -o $@

%.o : %.c
	$(CC) $(CFLAGS) $< -c -o $@

clean :
	rm -f *.o $(TARGET).bin $(TARGET).elf $(TARGET).dis


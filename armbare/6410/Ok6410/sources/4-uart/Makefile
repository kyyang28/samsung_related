
CROSS_COMPILE = arm-linux-

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

COBJS = start.o init.o uart.o
COBJS += myuart.o

CFLAGS = -Wall -DENABLE_UART_FIFO -DBOOT_FROM_NAND
LDFLAGS = -Ttext 0x0c000000
TARGET = uart

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


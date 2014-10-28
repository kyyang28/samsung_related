
CROSS_COMPILE := arm-linux-

CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump

COBJS := start.o clock_asm_init.o sdram_asm_init.o nand_ops.o uart_ops.o uart_main.o
CFLAGS := -Wall -DENABLE_UART_FIFO
LDFLAGS := -Tuart.lds
TARGET := uart_nand_mini2440

$(TARGET).bin : $(COBJS)
	$(LD) $(LDFLAGS) $^ -o $(TARGET).elf
	$(OBJCOPY) -O binary $(TARGET).elf $@
	$(OBJDUMP) -D $(TARGET).elf > $(TARGET).dis

%.o : %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET).elf $(TARGET).bin $(TARGET).dis *.o


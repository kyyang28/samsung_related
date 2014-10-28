
CROSS_COMPILE := arm-linux-

CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump

COBJS := start.o clock_asm_init.o sdram_asm_init.o nand_ops.o 
COBJS += leds_ops.o
COBJS += key_irq.o
COBJS += key_int_main.o
CFLAGS := -Wall
LDFLAGS := -Tkey_int.lds
TARGET := key_int_sdram_nand_mini2440

$(TARGET).bin : $(COBJS)
	$(LD) $(LDFLAGS) $^ -o $(TARGET).elf
	$(OBJCOPY) -O binary $(TARGET).elf $@
	$(OBJDUMP) -D $(TARGET).elf > $(TARGET).dis

%.o : %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET).bin $(TARGET).elf $(TARGET).dis *.o


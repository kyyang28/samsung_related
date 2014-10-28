
CROSS_COMPILER := arm-linux-

CC := $(CROSS_COMPILER)gcc
LD := $(CROSS_COMPILER)ld
OBJCOPY := $(CROSS_COMPILER)objcopy
OBJDUMP := $(CROSS_COMPILER)objdump

COBJS := start.o clock_asm_init.o sdram_asm_init.o 
COBJS += nand_ops.o L3_ops.o i2s_ops.o uda1341_ops.o audiodrv_ops.o leds_ops.o cache_ops.o
COBJS += i2s_main.o
CFLAGS := -Wall -DDMA_ENABLED #-DICACHE_ENABLED
LDFLAGS := -Ti2s.lds
TARGET := i2s_nand_mini2440

$(TARGET).bin : $(COBJS)
	$(LD) $(LDFLAGS) $^ -o $(TARGET).elf
	$(OBJCOPY) -O binary $(TARGET).elf $@
	$(OBJDUMP) -D $(TARGET).elf > $(TARGET).dis

%.o : %.S
	$(CC) $(CFLAGS) -c -o $@ $<

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f $(TARGET).* *.o



CROSS_COMPILER := arm-linux-

CC		:= $(CROSS_COMPILER)gcc
LD      := $(CROSS_COMPILER)ld
OBJCOPY := $(CROSS_COMPILER)objcopy
OBJDUMP := $(CROSS_COMPILER)objdump

CFLAGS := -Wall 
LDFLAGS := -Tlcd.lds

COBJS := start.o clock_asm_init.o sdram_asm_init.o 
COBJS += nand_ops.o lcd_ops.o fourleaf.o flower.o
COBJS += lcd_main.o
TARGET := lcd_nand_mini2440

$(TARGET).bin : $(COBJS)
	$(LD) $(LDFLAGS) $^ -o $(TARGET).elf
	$(OBJCOPY) -O binary $(TARGET).elf $@
	$(OBJDUMP) -D $(TARGET).elf > $(TARGET).dis

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o : %.S
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f $(TARGET).bin $(TARGET).elf $(TARGET).dis *.o


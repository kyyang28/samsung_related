
CROSS_COMPILER := arm-linux-

CC := $(CROSS_COMPILER)gcc
LD := $(CROSS_COMPILER)ld
OBJCOPY := $(CROSS_COMPILER)objcopy
OBJDUMP := $(CROSS_COMPILER)objdump

COBJS := start.o clock_asm_init.o sdram_asm_init.o nand_ops.o uart_ops.o leds_buzzer_ops.o 
COBJS += adc_ops.o printf.o adc_main.o
CFLAGS := -Wall
LDFLAGS := -Tadc.lds
TARGET := adc_nand_mini2440

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


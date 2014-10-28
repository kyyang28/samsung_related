
CROSS_COMPILE := arm-linux-

CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump

INCLUDEDIR := $(shell pwd)/include
COBJS := start.o clock_asm_init.o sdram_asm_init.o nand_ops.o uart_ops.o
COBJS += leds_ops.o buzzer_ops.o rtc_ops.o ts_ops.o i2c_ops.o eeprom_ops.o
COBJS += key_irq.o timer0_irq.o rtc_irq.o ts_irq.o i2c_irq.o
COBJS += printf.o lib_ops.o
COBJS += int_irq_vectors.o int_vector_all_main.o
CFLAGS := -Wall -nostdinc -I$(INCLUDEDIR)
LDFLAGS := -Tint_vector_all.lds
TARGET := int_vector_all_nand_mini2440

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


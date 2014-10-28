

CROSS_COMPILER := arm-linux-

CC := $(CROSS_COMPILER)gcc
LD := $(CROSS_COMPILER)ld
OBJCOPY := $(CROSS_COMPILER)objcopy
OBJDUMP := $(CROSS_COMPILER)objdump
BINSCRIPT := ./bin.sh 

COBJS := start.o leds_ops.o leds_main.o
CFLAGS := -Wall -Os
LDFLAGS := -Ttext 0xD0020010
TARGET := leds_sd_tiny210_semi

$(TARGET).bin : $(COBJS)
	$(LD) $(LDFLAGS) $^ -o $(TARGET).elf
	$(OBJCOPY) -O binary $(TARGET).elf $@
	$(OBJDUMP) -D $(TARGET).elf > $(TARGET).dis
	$(BINSCRIPT)

%.o : %.S
	$(CC) $(CFLAGS) -c -o $@ $<

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET).* *.o leds_sd_tiny210.bin


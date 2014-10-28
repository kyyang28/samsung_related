
CROSS_COMPILE := arm-linux-

CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump

COBJS := start.o 
COBJS += sdram_asm_init.o
COBJS += sdram_clk_leds_main.o
CFLAGS := -Wall
LDFLAGS := -Ttext 0x30000000
TARGET := sdram_clk_leds_nand_mini2440

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


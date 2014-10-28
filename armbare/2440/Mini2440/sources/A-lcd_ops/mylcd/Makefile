
CROSS_COMPILE := arm-linux-

CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump

COBJS := start.o clock_asm_init.o sdram_asm_init.o nand_ops.o lcd_ops.o lcd_main.o
CFLAGS := -Wall
LDFLAGS := -Tlcd.lds
#LIBPATH     := -L/home/kyyang/yan/cross_compiler/usr/local/arm/4.3.2/lib/gcc/arm-none-linux-gnueabi/4.3.2 
#LIBPATH     += -lgcc
TARGET := lcd_nand_mini2440

$(TARGET).bin : $(COBJS)
	$(LD) $(LDFLAGS) $^ -o $(TARGET).elf #$(LIBPATH)
	$(OBJCOPY) -O binary $(TARGET).elf $@
	$(OBJDUMP) -D $(TARGET).elf > $(TARGET).dis

%.o : %.S
	$(CC) $(CFLAGS) -c -o $@ $<

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET).* *.o


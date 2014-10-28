
CROSS_COMPILER := arm-linux-

AR      := $(CROSS_COMPILER)ar
CC      := $(CROSS_COMPILER)gcc
LD      := $(CROSS_COMPILER)ld
OBJCOPY := $(CROSS_COMPILER)objcopy
OBJDUMP := $(CROSS_COMPILER)objdump

INCLUDEDIR := $(shell pwd)/include
CFLAGS := -Wall 
CPPFLAGS := -nostdinc -I$(INCLUDEDIR)
LDFLAGS := -Tsd.lds 
LIBPATH := -L/home/kyyang/yan/cross_compiler/usr/local/arm/4.3.2/lib/gcc/arm-none-linux-gnueabi/4.3.2 
LIBPATH += -lgcc

export AR CC LD OBJCOPY OBJDUMP INCLUDEDIR CFLAGS CPPFLAGS

COBJS := start.o clock_asm_init.o sdram_asm_init.o 
COBJS += nand_ops.o uart_ops.o sd_ops.o lib_ops.o 
COBJS += sd_irq.o
COBJS += sd_main.o
COBJS += lib/libc.a
TARGET := sd_irq_nand_mini2440

$(TARGET).bin : $(COBJS)
	$(LD) $(LDFLAGS) $^ -o $(TARGET).elf $(LIBPATH)
	$(OBJCOPY) -O binary $(TARGET).elf $@
	$(OBJDUMP) -D $(TARGET).elf > $(TARGET).dis

.PHONY: lib/libc.a
lib/libc.a:
	cd lib; make; cd ..

%.o : %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

%.o : %.S
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f $(TARGET).bin $(TARGET).elf $(TARGET).dis *.o lib/*.o lib/*.a


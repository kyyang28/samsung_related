
CROSS_COMPILER := arm-linux-

AR      := $(CROSS_COMPILER)ar
CC      := $(CROSS_COMPILER)gcc
LD      := $(CROSS_COMPILER)ld
OBJCOPY := $(CROSS_COMPILER)objcopy
OBJDUMP := $(CROSS_COMPILER)objdump

INCLUDEDIR := $(shell pwd)/include
CFLAGS := -Wall -DENABLE_UART_FIFO
CPPFLAGS := -nostdinc -I$(INCLUDEDIR)
LDFLAGS := -Trtc.lds

export AR CC LD OBJCOPY OBJDUMP INCLUDEDIR CFLAGS CPPFLAGS

COBJS := start.o clock_asm_init.o sdram_asm_init.o nand_ops.o rtc_irq.o uart_ops.o rtc_ops.o rtc_main.o
COBJS += lib/libc.a
TARGET := rtc_alarm_sdram_nand_mini2440

$(TARGET).bin : $(COBJS)
	$(LD) $(LDFLAGS) $^ -o $(TARGET).elf
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


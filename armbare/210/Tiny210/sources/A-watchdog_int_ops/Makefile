
CROSS_COMPILER	= arm-linux-

CC      = $(CROSS_COMPILER)gcc
LD      = $(CROSS_COMPILER)ld
AR      = $(CROSS_COMPILER)ar
OBJCOPY = $(CROSS_COMPILER)objcopy
OBJDUMP = $(CROSS_COMPILER)objdump

INCLUDEDIR 	:= $(shell pwd)/include
CFLAGS 		:= -Wall -Os -fno-builtin -DUART0 
CPPFLAGS   	:= -nostdinc -I$(INCLUDEDIR)
LDFLAGS		:= -Tinterrupt.lds 

export 	CC AR LD OBJCOPY OBJDUMP INCLUDEDIR CFLAGS CPPFLAGS 

OBJS := start.o clock_asm_init.o ddr2_asm_init.o
# Put wdt_int_main.o before wdt_ops.o and interrupt.o in order to do the leds_init_ll firstly
OBJS += leds_ops.o uart_ops.o wdt_int_main.o wdt_ops.o interrupt.o 
OBJS += lib/libc.a

TARGET := wdt_int_sd_tiny210_semi

$(TARGET).bin: $(OBJS)
	${LD} $(LDFLAGS) -o $(TARGET).elf $^
	${OBJCOPY} -O binary -S $(TARGET).elf $@
	${OBJDUMP} -D $(TARGET).elf > $(TARGET).dis
	./bin.sh

.PHONY : lib/libc.a
lib/libc.a:
	cd lib; make; cd ..
	
%.o:%.c
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%.o:%.S
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	make  clean -C lib
	rm -f *.bin *.elf *.dis *.o
	

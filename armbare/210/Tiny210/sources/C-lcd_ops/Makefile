
CROSS_COMPILER	= arm-linux-

CC      = $(CROSS_COMPILER)gcc
LD      = $(CROSS_COMPILER)ld
AR      = $(CROSS_COMPILER)ar
OBJCOPY = $(CROSS_COMPILER)objcopy
OBJDUMP = $(CROSS_COMPILER)objdump

INCLUDEDIR 	:= $(shell pwd)/include
CFLAGS 		:= -Wall -Os -fno-builtin -DUART0 
CPPFLAGS   	:= -nostdinc -I$(INCLUDEDIR)
LDFLAGS		:= -Tlcd.lds 

export 	CC AR LD OBJCOPY OBJDUMP INCLUDEDIR CFLAGS CPPFLAGS 

OBJS := start.o clock_asm_init.o ddr2_asm_init.o
OBJS += uart_ops.o lcd_ops.o lcd_main.o
OBJS += lib/libc.a

TARGET := lcd_sd_tiny210_semi

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
	


CROSS_COMPILER	= arm-linux-

CC      = $(CROSS_COMPILER)gcc
LD      = $(CROSS_COMPILER)ld
AR      = $(CROSS_COMPILER)ar
OBJCOPY = $(CROSS_COMPILER)objcopy
OBJDUMP = $(CROSS_COMPILER)objdump

INCLUDEDIR 	:= $(shell pwd)/include
CFLAGS 		:= -Wall -Os -fno-builtin
CPPFLAGS   	:= -nostdinc -I$(INCLUDEDIR)
LDFLAGS		:= -Tuart_printf.lds

export 	CC AR LD OBJCOPY OBJDUMP INCLUDEDIR CFLAGS CPPFLAGS 

OBJS := start.o clock_asm_init.o uart_ops.o clock_uart_main.o lib/libc.a

TARGET := clock_uart_printf_sd_tiny210_semi

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
	

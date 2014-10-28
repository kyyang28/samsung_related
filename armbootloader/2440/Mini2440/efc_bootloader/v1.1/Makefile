
CC      = arm-linux-gcc
LD      = arm-linux-ld
AR      = arm-linux-ar
OBJCOPY = arm-linux-objcopy
OBJDUMP = arm-linux-objdump

CFLAGS 		:= -Wall -O2
CPPFLAGS   	:= -nostdinc -fno-builtin

OBJS := start.o boot_init.o board.o system.o string.o
TARGET := efc_boot_v1.1

$(TARGET).bin: $(OBJS)
	${LD} -Tefc_boot.lds -o $(TARGET).elf $^
	${OBJCOPY} -O binary -S $(TARGET).elf $@
	${OBJDUMP} -D -m arm $(TARGET).elf > $(TARGET).dis
	
%.o:%.c
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%.o:%.S
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.bin *.elf *.dis *.o
	

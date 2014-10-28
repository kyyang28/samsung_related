
CC      = arm-linux-gcc
LD      = arm-linux-ld
OBJCOPY = arm-linux-objcopy
OBJDUMP = arm-linux-objdump

OBJS := start.o lowlevel_init.o init.o led.o timer0_handler.o main.o
OBJS += irq.o

TARGET := timer0_interrupt_vector_12MHz

CFLAGS := -Wall -c

$(TARGET).bin: $(OBJS)
	$(LD) -Ttimer0.lds -o $(TARGET).elf $^
	$(OBJCOPY) -O binary -S $(TARGET).elf $@
	$(OBJDUMP) -D -m arm $(TARGET).elf > $(TARGET).dis
	ls -l $@

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET).bin $(TARGET).elf $(TARGET).dis *.o


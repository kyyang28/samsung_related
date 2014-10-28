OBJS := start.o ledlib.o led_syscall.o main.o
CFLAGS := -Wall -g 
TARGET := led_swi

$(TARGET).bin: $(OBJS)
	arm-linux-ld -Ttext 0x0 -o $(TARGET).elf $^
	arm-linux-objcopy -O binary -S $(TARGET).elf $@
	arm-linux-objdump -D -m arm $(TARGET).elf > $(TARGET).dis
	ls -l $@

%.o: %.c
	arm-linux-gcc $(CFLAGS) -c -o $@ $<

%.o: %.S
	arm-linux-gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET).bin $(TARGET).elf $(TARGET).dis *.o


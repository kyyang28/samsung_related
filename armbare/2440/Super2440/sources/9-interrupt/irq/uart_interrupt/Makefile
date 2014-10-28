OBJS := start.o init.o uart.o int_handler.o main.o
TARGET := uart_interrupt

$(TARGET).bin: $(OBJS)
	arm-linux-ld -Ttext 0x00000000 -o $(TARGET).elf $^
	arm-linux-objcopy -O binary -S $(TARGET).elf $@
	arm-linux-objdump -D -m arm $(TARGET).elf > $(TARGET).dis
	ls -l $@

%.o:%.c
	arm-linux-gcc -Wall -c -o $@ $<

%.o:%.S
	arm-linux-gcc -Wall -c -o $@ $<

clean:
	rm -f $(TARGET).bin $(TARGET).elf $(TARGET).dis *.o


OBJS := start.o init.o uart.o main.o

TARGET := uart_50MHz

$(TARGET).bin:	$(OBJS)
	arm-linux-ld -Tuart.lds -o $(TARGET).elf $^
	arm-linux-objcopy -O binary -S $(TARGET).elf $@
	arm-linux-objdump -D -m arm $(TARGET).elf > $(TARGET).dis
	ls -l $(TARGET).bin

%.o: %.c
	arm-linux-gcc -g -Wall -c -o $@ $<

%.o: %.S
	arm-linux-gcc -g -Wall -c -o $@ $<

clean:
	rm -f $(TARGET).* *.o


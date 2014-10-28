OBJS := start.o init.o key_handler.o led.o main.o
OBJS += irq.o
TARGET := key_interrupt_yaimp

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



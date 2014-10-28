OBJS := start.o lowlevel_init.o nand.o main.o
TARGET := nand_12MHz

$(TARGET).bin : $(OBJS)
	arm-linux-ld -Tnand.lds	-o $(TARGET).elf $^
	arm-linux-objcopy -O binary -S $(TARGET).elf $@
	arm-linux-objdump -D -m arm  $(TARGET).elf > $(TARGET).dis
	ls -l $@
	
%.o:%.c
	arm-linux-gcc -Wall -c -Wall -o $@ $<

%.o:%.S
	arm-linux-gcc -Wall -c -Wall -o $@ $<

clean:
	rm -f  $(TARGET).* *.o


#SRC  :=	$(shell ls *.c)
#OBJS := $(patsubst %.c, %.o, $(SRC))
OBJS := start.o init.o led.o relocate.o main.o
TARGET := led_sdram

$(TARGET).bin:	$(OBJS)
	arm-linux-ld -Ttext 0x30000000 -o $(TARGET).elf $^
	arm-linux-objcopy -O binary -S $(TARGET).elf $@
	arm-linux-objdump -D -m arm $(TARGET).elf > $(TARGET).dis
	ls -l $(TARGET).bin

%.o: %.c
	arm-linux-gcc -Wall -g -c $< -o $@
	
%.o: %.S
	arm-linux-gcc -Wall -g -c $< -o $@

clean:
	rm -f $(TARGET).* *.o


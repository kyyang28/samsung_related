OBJS := start.o main.o

led.bin: $(OBJS)
	arm-linux-ld -Tled.lds -o led.elf $^
	arm-linux-objcopy -O binary -S led.elf $@
	arm-linux-objdump -D -m arm  led.elf > led.dis
	ls -l led.bin

%.o:%.c
	arm-linux-gcc -Wall -g -c -o $@ $<

%.o:%.S
	arm-linux-gcc -Wall -g -c -o $@ $<

clean:
	rm -f   led.dis led.bin led.elf *.o

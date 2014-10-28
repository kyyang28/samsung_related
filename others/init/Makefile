
objs := init.o

all: $(objs)
	arm-linux-ld -Ttext 0x0000000 -o init.elf $^
	arm-linux-objcopy -O binary -S init.elf init.bin
	arm-linux-objdump -D -m arm init.elf > init.dis
	
%.o:%.c
	arm-linux-gcc -Wall -g -c -o $@ $<

%.o:%.S
	arm-linux-gcc -Wall -g -c -o $@ $<

clean:
	rm -f *.o init.elf init.dis

distclean:
	rm -f init.bin init.elf init.dis *.o
	
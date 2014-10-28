PRJ := watchdog

OBJS := start.S watchdog.c

CFLAGS := -Wall -Wstrict-prototypes -g -fomit-frame-pointer -ffreestanding

all : 	$(OBJS)
	arm-linux-gcc $(CFLAGS) -c -o start.o start.S
	arm-linux-gcc $(CFLAGS) -c -o watchdog.o watchdog.c
	arm-linux-ld -Twatchdog.lds start.o watchdog.o -o $(PRJ).elf
	arm-linux-objcopy -O binary -S $(PRJ).elf $(PRJ).bin
	arm-linux-objdump -D -m arm $(PRJ).elf > $(PRJ).dis
	ls -l $(PRJ).bin
clean :
	rm $(PRJ).dis $(PRJ).bin $(PRJ).elf *.o


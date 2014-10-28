OBJS := start.o lowlevel_init.o init.o main.o
TARGET := mmu

CFLAGS := -Wall -c
CCFLAGS += $(CFLAGS) #-DENABLE_CACHABLE_AND_BUFFERABLE

$(TARGET).bin : $(OBJS)
	arm-linux-ld -Tmmu.lds -o $(TARGET).elf $^
	arm-linux-objcopy -O binary -S $(TARGET).elf $@
	arm-linux-objdump -D -m arm $(TARGET).elf > $(TARGET).dis
	ls -l $@

%.o:%.c
	arm-linux-gcc $(CCFLAGS) -o $@ $<

%.o:%.S
	arm-linux-gcc $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET).bin $(TARGET).elf $(TARGET).dis *.o		
	

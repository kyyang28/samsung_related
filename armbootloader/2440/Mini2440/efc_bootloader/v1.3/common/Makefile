
OBJS := cmd_help.o 
OBJS += cmd_reset.o 
OBJS += cmd_boot.o 
OBJS += cmd_menu.o 
OBJS += cmd_serial.o
OBJS += cmd_mem.o
OBJS += cmd_go.o
OBJS += cmd_leds.o
OBJS += cmd_keyled.o
#OBJS += cmd_nand.o
OBJS += main.o

cmd.a: $(OBJS)
	${AR} -r -o $@ $^
	
%.o:%.c
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%.o:%.S
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -f cmd.a *.o
	

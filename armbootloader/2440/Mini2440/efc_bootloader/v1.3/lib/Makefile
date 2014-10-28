
OBJS := div64.o lib1funcs.o ctype.o muldi3.o printf.o string.o vsprintf.o
OBJS += board.o #bootm.o

libc.a: $(OBJS)
	${AR} -r -o $@ $^
	
%.o:%.c
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%.o:%.S
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -f libc.a *.o
	

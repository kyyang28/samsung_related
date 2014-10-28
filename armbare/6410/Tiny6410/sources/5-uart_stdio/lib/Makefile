OBJS := div64.o lib1funcs.o ctype.o muldi3.o printf.o string.o vsprintf.o

libc.a : $(OBJS)
	$(AR) -r -o $@ $^
	
%.o : %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean :
	rm -f libc.a *.o		
	

SECTIONS { 
  first     0x00000000 : { start.o lowlevel_init.o init.o }
  second    0xB0004000 : AT(2048) { main.o }
} 
 

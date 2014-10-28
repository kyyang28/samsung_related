SECTIONS { 
  first  	0x00000000 : { start.o lowlevel_init.o clock.o nand.o}
  second 	0x30000000 : AT(4096) { main.o }
} 
 



#include "../system/console.h"
#include "../lib/vsprintf.h"

int do_md(int argc, char *argv[]) 
{
	unsigned long addr;
	unsigned long *p;
	int i, j;
	
	if(argc != 2 || argv[1][0] != '0' || argv[1][1] != 'x') {
		put_string("\nUsage:   md 0xXXXXXXXX\n\n"); 
        return -1;
	}
	
	addr = simple_strtoul(argv[1], NULL, 16);
	p = (unsigned long *)addr;
	
	put_string("\n");
	for(i = 0; i < 16; ++i) {
		//put_string("0x");
		putx(addr + i*16);
		put_string(": ");
		for(j = 0; j < 4; ++j) {
			putx(*p++);
			put_string(" ");
		}
		put_string("\n");
	}
    
	put_string("\n");
    return 0;
}

#if 0
void do_mw(int argc, char *argv[]) {
		int addr;
		int value;
		int *p;
		
		if(argc != 3 || argv[1][0] != '0' || argv[1][1] != 'x' || argv[2][0] != '0' || argv[1][1] != 'x') {
				put_string("\nUsage:   mw 0xXXXXXXXX 0xXX\n\n"); return;
		}
		addr = my_atoi(argv[1]);
		value = my_atoi(argv[2]);
		p = (int *)addr;
		*p = value;
}
#endif


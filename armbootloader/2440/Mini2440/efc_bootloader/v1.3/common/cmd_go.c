
#include "../system/console.h"
#include "../lib/vsprintf.h"

unsigned long go_addr   = 0x32000000;   /* Default Go Address */

int do_go(int argc, char *argv[]) 
{
	unsigned long addr = go_addr;
	void (*entry_go)(void);
	
	if(argc > 2) {
		put_string("Usage:    go default_addr or go 0xXXXXXXXX, default_addr is 0x51000000\n");
	}
	
	if(argc == 2) {
		addr = simple_strtoul(argv[1], NULL, 16);		
	}
	
	entry_go = (void (*)(void))addr;
	entry_go();

	/* NOTREACHED */
    return 0;
}


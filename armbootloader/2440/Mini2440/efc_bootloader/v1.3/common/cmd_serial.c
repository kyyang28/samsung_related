

#include <stdio.h>
#include "../system/console.h"
#include "../kermit.h"
#include "../lib/vsprintf.h"

unsigned long load_addr = 0x32000000;	 /* Default Load Address */

int do_loadb(int argc, char *argv[]) 
{
	unsigned long offset = load_addr;
    unsigned long addr;    
	int rcode = 0;
	
	if(argc > 2) {
		put_string("Usage:    loadb default_addr or loadb 0xXXXXXXXX, default_addr is 0x32000000\n");	
	}
	
	if(argc == 2) {
		offset = simple_strtoul(argv[1], NULL, 16);
		printf ("## Ready for binary (kermit) download "
			"to 0x%08lX at %d bps...\r\n",
			offset,
			115200);
		addr = load_serial_bin (offset);

        if (addr == ~0) {
			load_addr = 0;
			printf ("## Binary (kermit) download aborted\r\n");
			rcode = 1;
		} else {
			printf ("## Start Addr      = 0x%08lX\r\n", addr);
			load_addr = addr;
		}
        return rcode;
	}
    
    printf ("## Ready for binary (kermit) download "
        "to 0x%08lX at %d bps...\r\n",
        offset,
        115200);
    addr = load_serial_bin (offset);
    
    if (addr == ~0) {
        load_addr = 0;
        printf ("## Binary (kermit) download aborted\r\n");
        rcode = 1;
    } else {
        printf ("\r\n## Start Addr      = 0x%08lX\r\n", addr);
        load_addr = addr;
    }

    return rcode;
}


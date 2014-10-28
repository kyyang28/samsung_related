
#include <stdio.h>

int do_help(int argc, char *argv[]) 
{
	printf("    ?           -   alias for 'help'\r\n");	
	printf("    help        -   help\r\n");
	printf("    reset       -   reset the board\r\n");    
	printf("    boot        -   boot linux kernel\r\n");
	printf("    go          -   run a program at the specific address\r\n");
	printf("    loadb       -   kermit download\r\n");
	printf("    md          -   display the memory contents\r\n");
	printf("    menu        -   show the params menu\r\n");
	printf("    hwtest      -   hardware testing programs\r\n");
    return 0;
}


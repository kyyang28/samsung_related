
#include <stdio.h>
#include "../include/string.h"
#include "../system/console.h"
#include "command.h"

#define BOOTDELAY                   (5)
#define MY_PROMPT	                "Pete&Charles # "

extern void do_bootm_linux(void);
extern void serial_putc(unsigned char c);
extern unsigned char serial_getc(void);
extern void serial_puts(char *s);
extern void showParamsMenu(void);
extern int serial_tstc(void);

static int bootdelay = BOOTDELAY;


__cmd_struct cmd_array[] = {
    { "boot",   do_boot_linux },
    { "?",      do_help },
	{ "help",   do_help },
    { "loadb",  do_loadb },
    { "go",     do_go },
    { "reset",  do_reset },
    { "md",     do_md },
    { "menu",   do_menu },
    { "hwtest", do_hwtest },
#if 0
	{ "mw", do_mw },
	{ "nandread", do_nand_read },
	{ "nandprogram", do_nand_program }
#endif
};

static const int CMD_SIZE = sizeof(cmd_array) / sizeof(__cmd_struct);

static void command_parse(char *buf, int *argc, char *argv[]) 
{
	char ch;
	int status = 0;
	int i = 0;
	*argc = 0;
	
	while( (ch = buf[i]) != '\0') {
		if( ch != ' ' && status == 0 ) {
			status = 1;
			argv[(*argc)++] = buf + i;
		}
		
		if( ch == ' ' && status == 1 ) {
			status = 0;
			buf[i] = '\0';
		}
		i++;
	}
} 

int run_command(char *cmd) 
{
	int j;    
	int argc = 0;
	char *argv[CMD_SIZE];

    command_parse(cmd, &argc, argv);
    
	for(j = 0; j < CMD_SIZE; ++j) {
    	if(strcmp(argv[0], cmd_array[j].cmd_name) == 0) {
			cmd_array[j].cmd_func(argc, argv);
			return 0;
    	}
	}
	return -1;
}

int tstc(void)
{
    return serial_tstc();
}

void delay (unsigned long ticks)
{
    int i, j;
	for (i = 0; i < ticks; i++) 
		for (j = 0; j < 10000; j++) 
			;     
}

/*
 * Reads and returns a character from the serial port
 *   - Times out after delay iterations checking for presence of character
 *   - Sets *error_p to UART error bits or - on timeout
 *   - On timeout, sets *error_p to -1 and returns 0
 */
char awaitkey(unsigned long dly)
{
    int i;

    if (dly == -1) {
        while (1) {
            if (tstc()) /* we got a key press	*/
                return serial_getc();
        }
    }else {        
        for (i = 0; i < dly; i++) {
    		if (tstc()) /* we got a key press	*/
    			return serial_getc();
            delay(10*1000);
        }
    }
    
    return 0;
}

void efc_shell(void)
{
    char buf[100];

	while(1) {
    	printf(MY_PROMPT);
    	get_string(buf);
    	if(run_command(buf) == -1 && strcmp(buf, "\0"))
    		put_string("    Unknown command - try \"help\" or \"?\" to get further instruction\n");
	}

    printf("\r\n");
}

static int abortboot(int bootdelay)
{
    int abort = 0;

	printf("Hit any key to stop autoboot: %2d ", bootdelay);

    if (bootdelay >= 0) {
        if (tstc()) {
            (void)serial_getc();    /* consume input */
            serial_puts("\b\b\b 0");
            abort = 1;
        }
    }

    while ((bootdelay > 0) && (!abort)) {
        int i;

        --bootdelay;
        /* delay 100 * 10ms */
        for (i = 0; !abort && i < 0x700000; ++i) {
            if (tstc()) {   /* we got a key press   */
                abort  = 1; /* don't auto boot  */
                bootdelay = 0;  /* no more delay    */

                (void)serial_getc();  /* consume input    */
                break;
            }
            delay(100000);
        }
        
		printf("\b\b\b%2d ", bootdelay);        
    }

    serial_puts("\r\n\r\n");

    return abort;
}

void main_loop(void)
{       
    if (bootdelay >= 0 && !abortboot (bootdelay)) {
        /* Boot linux kernel */
        do_bootm_linux();    
    }

    showParamsMenu();    
    efc_shell();
}


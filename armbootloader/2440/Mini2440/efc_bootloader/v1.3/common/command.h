
#ifndef __COMMAND_H
#define __COMMAND_H

typedef struct {
	char cmd_name[20];
	int  (*cmd_func)(int argc, char *argv[]);
}__cmd_struct;


int do_help(int argc, char *argv[]);
int do_reset(int argc, char *argv[]);
int do_boot_linux(int argc, char *argv[]);
int do_go(int argc, char *argv[]);
int do_loadb(int argc, char *argv[]);
int do_md(int argc, char *argv[]);
int do_menu(int argc, char *argv[]);
int do_hwtest(int argc, char *argv[]);

#if 0
void do_mw(int argc, char *argv[]);
void do_nand_read(int argc, char * argv [ ]);
void do_nand_program(int argc, char * argv [ ]);
#endif

#endif


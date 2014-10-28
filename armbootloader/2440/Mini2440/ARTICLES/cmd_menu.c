/*
 * Pete&Charles http://embeddedfansclub.taobao.com
 *
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <nand.h>
#include <search.h>
#include <errno.h>


extern char console_buffer[];
extern int readline (const char *const prompt);
//extern char awaitkey(unsigned long delay, int* error_p);
extern void download_nkbin_to_flash(void);
extern int isBootFromNorFlash(void);


/*
 * Reads and returns a character from the serial port
 *   - Times out after delay iterations checking for presence of character
 *   - Sets *error_p to UART error bits or - on timeout
 *   - On timeout, sets *error_p to -1 and returns 0
 */
char awaitkey(unsigned long delay, int* error_p)
{
    int i;
    char c;

    if (delay == -1) {
        while (1) {
            if (tstc()) /* we got a key press	*/
                return getc();
        }
    }
    else {        
        for (i = 0; i < delay; i++) {
    		if (tstc()) /* we got a key press	*/
    			return getc();
            udelay (10*1000);
        }
    }
    
    if (error_p)
        *error_p = -1;
    return 0;
}


/**
 * Parses a string into a number.  The number stored at ptr is
 * potentially suffixed with K (for kilobytes, or 1024 bytes),
 * M (for megabytes, or 1048576 bytes), or G (for gigabytes, or
 * 1073741824).  If the number is suffixed with K, M, or G, then
 * the return value is the number multiplied by one kilobyte, one
 * megabyte, or one gigabyte, respectively.
 *
 * @param ptr where parse begins
 * @param retptr output pointer to next char after parse completes (output)
 * @return resulting unsigned int
 */
static unsigned long memsize_parse2 (const char *const ptr, const char **retptr)
{
	unsigned long ret = simple_strtoul(ptr, (char **)retptr, 0);
    int sixteen = 1;

	switch (**retptr) {
		case 'G':
		case 'g':
			ret <<= 10;
		case 'M':
		case 'm':
			ret <<= 10;
		case 'K':
		case 'k':
			ret <<= 10;
			(*retptr)++;
            sixteen = 0;
		default:
			break;
	}

    if (sixteen)
        return simple_strtoul(ptr, NULL, 16);
    
	return ret;
}


void displayParamsMenuUsage()
{
    printf("\r\n@@@@@ Parameter Menu @@@@@\r\n");
    printf("[v] View the parameters\r\n");
    printf("[s] Set parameter \r\n");
    printf("[d] Delete parameter \r\n");
    printf("[w] Write the parameters to flash memeory \r\n");
    printf("[q] Quit \r\n");
    printf("Enter your selection: ");
}


void showParamsMenu(void)
{
    char c;
    char cmd_buf[256];
    char name_buf[20];
    char val_buf[256];
    
    while (1) {
        displayParamsMenuUsage();
        c = awaitkey(-1, NULL);
        printf("%c\n", c);
        switch (c) {
            case 'v':
                strcpy(cmd_buf, "printenv ");
                printf("Name(press \"enter\" to view all paramters): ");
                readline(NULL);
                strcat(cmd_buf, console_buffer);
                run_command(cmd_buf, 0);
                break;
            
            case 's':
                sprintf(cmd_buf, "setenv ");

                printf("Name: ");
                readline(NULL);
                strcat(cmd_buf, console_buffer);

                printf("Value: ");
                readline(NULL);
                strcat(cmd_buf, " ");
                strcat(cmd_buf, console_buffer);

                run_command(cmd_buf, 0);
                break;
            
            case 'd':
                sprintf(cmd_buf, "setenv ");

                printf("Name: ");
                readline(NULL);
                strcat(cmd_buf, console_buffer);

                run_command(cmd_buf, 0);
                break;
            
            case 'w':
                sprintf(cmd_buf, "saveenv");
                run_command(cmd_buf, 0);
                break;
            
            case 'q':
                return;
                break;
        }
    }
}


void displayMainMenuUsage(void)
{
    printf("\n");
    printf("+---------------------------------------------+\n");
    printf("|         Pete&Charles U-BOOT_2012.07         |\n");
    printf("+---------------------------------------------+\n");
    printf("\n");

    printf("[f] Format the Nand Flash\r\n");
    if (isBootFromNorFlash())
        printf("[n] Download u-boot to NORFLASH using TFTP protocol\r\n");
    printf("[u] Download u-boot to NANDFLASH using TFTP protocol\r\n");    
    printf("[k] Download kernel to NANDFLASH using TFTP protocol\r\n");    
    printf("[y] Download rootfs_yaffs to NANDFLASH using TFTP protocol\r\n");    
    printf("[j] Download rootfs_jffs2 to NANDFLASH using TFTP protocol\r\n");    
    printf("[p] Set the boot parameters\r\n");
    printf("[b] Boot the system from Nand Flash\r\n");
    printf("[r] Reset the u-boot\r\n");
    printf("[q] Quit to shell\r\n");
    printf("\nEnter your selection: ");
}


void showUBOOTMenu(void)
{
    char c;
    char cmd_buf[200];
    char *p = NULL;
    unsigned long size;
    unsigned long offset;
    char *filesize;
    char *endp;
    unsigned long filesizeVal;
    struct mtd_info *mtd = &nand_info[nand_curr_device];

    while (1) {
        displayMainMenuUsage();
        c = awaitkey(-1, NULL);
        printf("%c\n", c);
        switch (c) {
#if 0
			case 'n':
			{
                strcpy(cmd_buf, "usbslave 1 0x30000000; nand erase bootloader; nand write.jffs2 0x30000000 bootloader $(filesize)");
                run_command(cmd_buf, 0);
                break;
			}
            case 'o':
            {
                if (bBootFrmNORFlash())
                {
                    strcpy(cmd_buf, "usbslave 1 0x30000000; protect off all; erase 0 +$(filesize); cp.b 0x30000000 0 $(filesize)");
                    run_command(cmd_buf, 0);
                }
				break;
            }
            
            case 'k':
            {
                strcpy(cmd_buf, "usbslave 1 0x30000000; nand erase kernel; nand write.jffs2 0x30000000 kernel $(filesize)");
                run_command(cmd_buf, 0);
                break;
            }

            case 'j':
            {
                strcpy(cmd_buf, "usbslave 1 0x30000000; nand erase root; nand write.jffs2 0x30000000 root $(filesize)");
                run_command(cmd_buf, 0);
                break;
            }
#if 0
            case 'c':
            {
                strcpy(cmd_buf, "usbslave 1 0x30000000; nand erase root; nand write.jffs2 0x30000000 root $(filesize)");
                run_command(cmd_buf, 0);
                break;
            }
#endif
            case 'y':
            {
                strcpy(cmd_buf, "usbslave 1 0x30000000; nand erase root; nand write.yaffs 0x30000000 root $(filesize)");
                run_command(cmd_buf, 0);
                break;
            }

            case 'd':
            {
                extern volatile U32 downloadAddress;
                extern int download_run;
                
                download_run = 1;
                strcpy(cmd_buf, "usbslave 1");
                run_command(cmd_buf, 0);
                download_run = 0;
                sprintf(cmd_buf, "go %x", downloadAddress);
                run_command(cmd_buf, 0);
                break;
            }

			case 'z':
			{
				strcpy(cmd_buf, "usbslave 1 0x30008000");
				run_command(cmd_buf, 0);
				break;
			}

			case 'g':
			{
				extern void do_bootm_rawLinux (ulong addr);
				do_bootm_rawLinux(0x30008000);
			}
#endif
            case 'u':
                printf("\n");
                printf("+----------------------------------------------------------+\n");
                printf("|       Downloading u-boot to NANDFLASH(using tftp) ...    |\n");
                printf("+----------------------------------------------------------+\n");
                printf("\n");
                strcpy(cmd_buf, "tftp 30000000 u-boot.bin; nand erase.part u-boot; nand write 30000000 u-boot");
                run_command(cmd_buf, 0);
                break;

            case 'n':
                printf("\n");
                printf("+---------------------------------------------------------+\n");
                printf("|       Downloading u-boot to NORFLASH(using tftp) ...    |\n");
                printf("+---------------------------------------------------------+\n");
                printf("\n");
                strcpy(cmd_buf, "tftp 30000000 u-boot.bin; protect off all; erase 0 3FFFF; cp.b 30000000 0 40000");
                run_command(cmd_buf, 0);
                break;                

            case 'k':
                printf("\n");
                printf("+---------------------------------------------+\n");
                printf("|       Downloading kernel(using tftp) ...    |\n");
                printf("+---------------------------------------------+\n");
                printf("\n");
                strcpy(cmd_buf, "tftp 30000000 uImage;nand erase.part kernel;nand write 30000000 kernel");
                run_command(cmd_buf, 0);
                break;

            case 'y':
                printf("\n");
                printf("+---------------------------------------------+\n");
                printf("|   Downloading rootfs_yaffs(using tftp) ...  |\n");
                printf("+---------------------------------------------+\n");
                printf("\n");
                strcpy(cmd_buf, "tftp 30000000 rootfs.yaffs2;nand erase.part root");
                run_command(cmd_buf, 0);
#if 1
                filesize = getenv("filesize");
                filesizeVal = (unsigned long)simple_strtoul(filesize, &endp, 16);
                memset(cmd_buf, 0, sizeof(cmd_buf));
                sprintf(cmd_buf, "%s %x","nand write.yaffs 30000000 260000", filesizeVal);                 
                run_command(cmd_buf, 0);
#else
                filesize = getenv("filesize");                
                memset(cmd_buf, 0, sizeof(cmd_buf));
                strcpy(cmd_buf, strcat("nand write.yaffs 30000000 260000 ", filesize));
                run_command(cmd_buf, 0);
#endif
                break;

            case 'j':
                    printf("\n");
                    printf("+---------------------------------------------+\n");
                    printf("|   Downloading rootfs_jffs2(using tftp) ...  |\n");
                    printf("+---------------------------------------------+\n");
                    printf("\n");
                    strcpy(cmd_buf, "tftp 30000000 rootfs.jffs2;nand erase.part root");
                    run_command(cmd_buf, 0);

                    filesize = getenv("filesize");
                    filesizeVal = (unsigned long)simple_strtoul(filesize, &endp, 16);
                    memset(cmd_buf, 0, sizeof(cmd_buf));
                    sprintf(cmd_buf, "%s %x","nand write.jffs2 30000000 260000", filesizeVal);                 
                    run_command(cmd_buf, 0);
                    break;
                
            case 'b':
                printf("Booting Linux ...\n");
                strcpy(cmd_buf, "nand read 30000000 kernel; bootm 30000000");
                run_command(cmd_buf, 0);
                break;

            case 'f':
                strcpy(cmd_buf, "nand erase ");

                printf("Start address: ");
                readline(NULL);
                strcat(cmd_buf, console_buffer);

                printf("Size(eg. 4000000, 0x4000000, etc): ");
                readline(NULL);
                p = console_buffer;
                size = memsize_parse2(p, &p);
                sprintf(console_buffer, " %x", size);
                strcat(cmd_buf, console_buffer);

                run_command(cmd_buf, 0);
                break;
                
#if 1
            case 's':
                showParamsMenu();
                break;
#endif
            case 'r':
				run_command("reset", 0);
                break;
            
            case 'q':
                return;    
                break;

        }
                
    }
}

int do_menu (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    showUBOOTMenu();
    return 0;
}

U_BOOT_CMD(
	menu,	3,	0,	do_menu,
	"menu - display the menu, to select the booting options.",
	" - display the menu, to select the booting options."
);


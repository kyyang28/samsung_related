
/* 
 *  Embedded Fans Club 
 *  Bootloader 2nd stage
 *  c codes to do the rest of the parts.
 */

#include "setup.h"

#define NULL    0

#define BOOTLOADER_VERSION          "Version: 2012.09"
#define BOOTLOADER_VERSION_STRING   BOOTLOADER_VERSION " (" __DATE__ " - " __TIME__ ") "

#define BOOT_PARAMS_ADDR            (0x30000100)
#define LOAD_ADDR                   (0x30008000)
#define MACHINE_ID                  (362)

#define UIMAGE_OFFSET               (0x60000)
#define UIMAGE_HEADER_SIZE          (64)
#define ZIMAGE_OFFSET               (UIMAGE_OFFSET + UIMAGE_HEADER_SIZE)
#define KERNEL_SIZE                 (0x280000)

extern void nand_read_ll(unsigned int addr, unsigned char *buff, unsigned int size);
extern unsigned int efc_strlen(const char *str);
extern char * efc_strcpy(char *dest, const char *src);
extern void serial_puts(char *s);
extern void icache_enable(void);
extern int serial_init(void);


static struct tag *params;

static char commandline[] = \
            "noinitrd console=ttySAC0,115200 root=/dev/mtdblock3 rootfstype=yaffs";

static char version_string[] = BOOTLOADER_VERSION_STRING;

static void setup_start_tag (void)
{
	params = (struct tag *)BOOT_PARAMS_ADDR;

	params->hdr.tag = ATAG_CORE;
	params->hdr.size = tag_size (tag_core);

	params->u.core.flags = 0;
	params->u.core.pagesize = 0;
	params->u.core.rootdev = 0;

	params = tag_next (params);
}

static void setup_memory_tags(void)
{
	params->hdr.tag = ATAG_MEM;
	params->hdr.size = tag_size (tag_mem32);

	params->u.mem.start = 0x30000000;
	params->u.mem.size = 0x4000000;     /* 64 MiB */

	params = tag_next (params);
}

static void setup_commandline_tag(char *commandline)
{
	char *p;

	if (!commandline)
		return;

	/* eat leading white space */
	for (p = commandline; *p == ' '; p++);

	/* skip non-existent command lines so the kernel will still
	 * use its default command line.
	 */
	if (*p == '\0')
		return;
    
	params->hdr.tag = ATAG_CMDLINE;
	params->hdr.size = (sizeof (struct tag_header) + efc_strlen (p) + 1 + 4) >> 2;

	efc_strcpy (params->u.cmdline.cmdline, p);

	params = tag_next (params);
}

static void setup_end_tag(void)
{
	params->hdr.tag = ATAG_NONE;
	params->hdr.size = 0;
}

void hang(void)
{
	serial_puts("\r\n### ERROR ### Please RESET the board ###\r\n");
	for (;;);
}

int display_banner(void)
{
    serial_puts("\r\n\r\n");
    serial_puts(version_string);
    serial_puts("\r\n");
    return 0;
}

int display_efc_info(void)
{
    serial_puts("\r\n");
    serial_puts("+---------------------------------------------+\r\n");
    serial_puts("|              Embedded Fans Club             |\r\n");
    serial_puts("|       http://www.embeddedfansclub.com       |\r\n");
    serial_puts("|      http://embeddedfansclub.taobao.com     |\r\n");
    serial_puts("+---------------------------------------------+\r\n");
    serial_puts("\r\n");
    serial_puts("\r\n");
    return 0;
}

typedef int (init_func)(void);

init_func *init_sequence[] = {
    serial_init,
    display_banner,
    display_efc_info,
    NULL,
};

void board_init_r(void)
{
    init_func **init_func_ptr;
    
	void (*kernel_entry)(int zero, int arch, unsigned int params);
	kernel_entry = (void (*)(int, int, unsigned int))LOAD_ADDR;

    /* 
     *  Make sure the functions in init_sequence array is invoked successfully,
     *  otherwise, the system is halted.
     */
    for (init_func_ptr = init_sequence; *init_func_ptr; ++init_func_ptr) {
        if ((*init_func_ptr)() != 0) {
            hang();
        }
    }

    serial_puts("Loading kernel...\r\n");

    /* Enable icache */
    icache_enable();
    
    /* Load the kernel from Nandflash to SDRAM */
    /* KERNEL_SIZE = 0x280000 = 2.5Mib */
    nand_read_ll(ZIMAGE_OFFSET, (unsigned char *)LOAD_ADDR, KERNEL_SIZE);

    serial_puts("Setting boot parameters...\r\n");

    /* Setup the u-boot&kernel params */
    /* 1. start_tag */
    /* 2. memory_tags */
    /* 3. commandline_tag */
    /* 4. end_tag */
    setup_start_tag();
    setup_memory_tags();
    setup_commandline_tag(commandline);
    setup_end_tag();

    serial_puts("Starting kernel...\r\n");
    serial_puts("\r\n");

    /* Start the kernel */    
	kernel_entry(0, MACHINE_ID, BOOT_PARAMS_ADDR);

    /* Should never get this far!! */
    hang();
}


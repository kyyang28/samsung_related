
#include "setup.h"

extern void nand_read_ll(unsigned int addr, unsigned char *buff, unsigned int size);
extern unsigned int efc_strlen(const char *str);
extern char * efc_strcpy(char *dest, const char *src);
extern void puts(char *s);
extern void icache_enable(void);

static struct tag *params;

static void setup_start_tag (void)
{
	params = (struct tag *)0x30000100;

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


int armboot_start(void)
{
	void (*kernel_entry)(int zero, int arch, unsigned int params);
    
	kernel_entry = (void (*)(int, int, unsigned int))0x30008000;

    puts("\r\n");
    puts("\r\n");
    puts("+---------------------------------------------+\r\n");
    puts("|              Embedded Fans Club             |\r\n");
    puts("|       http://www.embeddedfansclub.com       |\r\n");
    puts("|      http://embeddedfansclub.taobao.com     |\r\n");
    puts("+---------------------------------------------+\r\n");
    puts("\r\n");

    puts("+---------------------------------------------+\r\n");
    puts("|         Pete&Charles Bootloader v1.0        |\r\n");
    puts("+---------------------------------------------+\r\n");
    puts("\r\n");

    puts("Loading kernel...\r\n");

    /* Enable icache */
    icache_enable();
    
    /* Load the kernel from Nandflash to RAM */
    /* 0x280000 = 2.5Mib */
    nand_read_ll(0x60000+64, (unsigned char *)0x30008000, 0x280000);

    puts("Setting boot parameters...\r\n");

    /* Setup the u-boot&kernel params */
    /* 1. start_tag */
    /* 2. memory_tags */
    /* 3. commandline_tag */
    /* 4. end_tag */
    setup_start_tag();
    setup_memory_tags();
    setup_commandline_tag("noinitrd console=ttySAC0,115200 root=/dev/mtdblock3 rootfstype=yaffs");
    setup_end_tag();

    puts("Starting kernel...\r\n");
    puts("\r\n");

    /* Start the kernel */    
	kernel_entry(0, 362, 0x30000100);

    /* Should never get this far!! */
    return -1;
}




#include "include/setup.h"
#include "string.h"

#define BOOT_PARAMS_ADDR            (0x30000100)
#define LOAD_ADDR                   (0x30008000)
#define MACHINE_ID                  (362)

#define UIMAGE_OFFSET               (0x60000)
#define UIMAGE_HEADER_SIZE          (64)
#define ZIMAGE_OFFSET               (UIMAGE_OFFSET + UIMAGE_HEADER_SIZE)
#define KERNEL_SIZE                 (0x280000)

extern void nand_read_ll(unsigned int addr, unsigned char *buff, unsigned int size);
extern void serial_puts(char *s);

static struct tag *params;

static char commandline[] = \
            "noinitrd console=ttySAC0,115200 root=/dev/mtdblock3 rootfstype=yaffs";

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
	params->hdr.size = (sizeof (struct tag_header) + strlen (p) + 1 + 4) >> 2;

	strcpy (params->u.cmdline.cmdline, p);

	params = tag_next (params);
}

static void setup_end_tag(void)
{
	params->hdr.tag = ATAG_NONE;
	params->hdr.size = 0;
}

static void boot_jump_linux(void)
{
	void (*kernel_entry)(int zero, int arch, unsigned int params);
	kernel_entry = (void (*)(int, int, unsigned int))LOAD_ADDR;

    serial_puts("\r\n");
    serial_puts("Loading kernel...\r\n");
    
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
}

extern void do_bootm_linux(void)
{
    boot_jump_linux();
}


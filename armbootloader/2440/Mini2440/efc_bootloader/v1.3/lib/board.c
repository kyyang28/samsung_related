
/* 
 *  Embedded Fans Club 
 *  Bootloader 2nd stage
 *  c codes to do the rest of the parts.
 */

#define NULL    0

#define BOOTLOADER_VERSION          "Version: 2012.09"
#define BOOTLOADER_VERSION_STRING   BOOTLOADER_VERSION " (" __DATE__ " - " __TIME__ ") "

extern void serial_puts(char *s);
extern int icache_enable(void);
extern int serial_init(void);
extern void main_loop(void);

static char version_string[] = BOOTLOADER_VERSION_STRING;

void hang(void)
{
	serial_puts("\r\n### ERROR ### Please RESET the board ###\r\n");
	for (;;);
}

static int display_banner(void)
{
    serial_puts("\r\n\r\n");
    serial_puts(version_string);
    serial_puts("\r\n");
    return 0;
}

static int display_efc_info(void)
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
    /* Enable icache */
    icache_enable,
    NULL,
};

void board_init_r(void)
{
    init_func **init_func_ptr;
    
    /* 
     *  Make sure the functions in init_sequence array is invoked successfully,
     *  otherwise, the system is halted.
     */
    for (init_func_ptr = init_sequence; *init_func_ptr; ++init_func_ptr) {
        if ((*init_func_ptr)() != 0) {
            hang();
        }
    }
    
	/* main_loop() can return to retry autoboot, if so just run it again. */
    for (;;) {
        main_loop();
    }
}


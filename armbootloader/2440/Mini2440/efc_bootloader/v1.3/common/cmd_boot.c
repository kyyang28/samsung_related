

extern void do_bootm_linux(void);

int do_boot_linux(int argc, char *argv[])
{
    do_bootm_linux();
    
	/* NOTREACHED */
    return 0;
}



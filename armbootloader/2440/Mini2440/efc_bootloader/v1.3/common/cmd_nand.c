

#if 0
void do_nand_read(int argc, char *argv[]) {
	unsigned int block, page;
	unsigned char *addr;

	if(argc < 4) {
		put_string("Usage:    nandread block page addr\n");
		return;
	}

	block = (unsigned int)my_atoi(argv[1]);
	page = (unsigned int)my_atoi(argv[2]);
	addr = (unsigned char *)my_atoi(argv[3]);
	nand_read_page(block, page, addr);
}


void do_nand_program(int argc, char *argv[]) {
	unsigned int block, page;
	unsigned char *addr;

	if(argc < 4) {
		put_string("Usage:    nandread block page addr\n");
		return;
	}

	block = (unsigned int)my_atoi(argv[1]);
	page = (unsigned int)my_atoi(argv[2]);
	addr = (unsigned char *)my_atoi(argv[3]);
	nand_program_page(block, page, addr);
}
#endif



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

//#define AT24CXX_USER_DBG    printf
#define AT24CXX_USER_DBG(...)

void printUsage(char *fileName)
{
	printf("[USER]%s read addr\n", fileName);
	printf("[USER]%s write addr val\n", fileName);
}

int main(int argc, char **argv)
{
	int fd;
	unsigned char buf[2];
	
	if ((argc != 3) && (argc != 4)) {
		printUsage(argv[0]);
		return -1;
	}

	fd = open("/dev/at24c08", O_RDWR);
	if (fd < 0) {
		printf("[USER]can't open /dev/at24c08\n");
		return -1;
	}

	if (strcmp(argv[1], "read") == 0) {
        AT24CXX_USER_DBG("read\n");
		buf[0] = strtoul(argv[2], NULL, 0);
		read(fd, buf, 1);
		printf("\n[USER]dataValue is char[%c], int[%d], hex[0x%2x]\n\n", buf[0], buf[0], buf[0]);
	}else if ((strcmp(argv[1], "write") == 0) && (argc == 4)) {
        AT24CXX_USER_DBG("write\n");
		buf[0] = strtoul(argv[2], NULL, 0);
		buf[1] = strtoul(argv[3], NULL, 0);
		if (write(fd, buf, 2) != 2) {          
			printf("\n[USER]Failed to write, addr = 0x%02x, data = 0x%02x\n\n", buf[0], buf[1]);
		}
	}else {
		printUsage(argv[0]);
		return -1;
	}
	
	return 0;
}


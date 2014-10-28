
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

/* 
 *	ledapp on 
 *	ledapp off 
 */
int main(int argc, char **argv)
{
	int fd, val;

	fd = open("/dev/led", O_RDWR);

	if(fd < 0) 
		printf("cannot open!\n");

	if(argc != 2) {
		printf("Usage: \n%s <on|off>\n", argv[0]); 
		return 0;
	}

	if(strcmp(argv[1], "on") == 0) {
		val = 0;
	}else if(strcmp(argv[1], "off") == 0) {
		val = 1;
	}

	write(fd, &val, 4);
	
	return 0;
}


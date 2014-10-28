
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>


int main(int argc, char **argv)
{
	int fd;
	unsigned char key_vals;

	fd = open("/dev/button_irq", O_RDWR);

	if(fd < 0) 
		printf("cannot open!\n");
	
	while(1) {
		read(fd, &key_vals, 1);
		printf("key_vals = 0x%x\n", key_vals);
	}
	
	return 0;
}



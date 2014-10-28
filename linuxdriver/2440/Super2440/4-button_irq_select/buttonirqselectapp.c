
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>


int main(int argc, char **argv)
{
	int fd;
	unsigned char key_vals;
	
	int ret;
	struct pollfd fds[1];

	fd = open("/dev/button_irq_poll", O_RDWR);

	if(fd < 0) 
		printf("cannot open!\n");

	
	fds[0].fd = fd;
	fds[0].events = POLLIN;
	while(1) {
		ret = poll(fds, 1, 5000);
		if(ret == 0) {
			printf("Time out!\n");			
		}else {
			read(fd, &key_vals, 1);
			printf("key_vals = 0x%x\n", key_vals);
		}
	}
	
	return 0;
}



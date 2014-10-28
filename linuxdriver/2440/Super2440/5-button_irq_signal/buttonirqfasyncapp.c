
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <poll.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

int fd;

void my_signal_func(int signum)
{
	unsigned char key_val;
	read(fd, &key_val, 1);
	printf("key_val = 0x%x\n", key_val);
}


int main(int argc, char **argv)
{
	int oflags;
	
	signal(SIGIO, my_signal_func); 

	fd = open("/dev/button_irq_signal", O_RDWR);
	if(fd < 0) 
		printf("cannot open!\n");

	fcntl(fd, F_SETOWN, getpid());
	oflags = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, oflags | FASYNC);
	
	while(1) {
		sleep(1000);
	}
	
	return 0;
}



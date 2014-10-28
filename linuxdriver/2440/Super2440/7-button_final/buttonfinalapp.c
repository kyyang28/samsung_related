
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <poll.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

int fd;
#if 0
void my_signal_func(int signum)
{
	unsigned char key_val;
	read(fd, &key_val, 1);
	printf("key_val = 0x%x\n", key_val);
}
#endif

int main(int argc, char **argv)
{
	//int oflags;
	
	//signal(SIGIO, my_signal_func); 
    int ret;
	unsigned char key_val;

	fd = open("/dev/buttonFinal", O_RDWR);
	if(fd < 0) {
		printf("cannot open!\n");
		return -1;
	}
	
	while(1) {
		ret = read(fd, &key_val, 1);
	    printf("key_val = 0x%x, ret = %d\n", key_val, ret);
        //sleep(5);
	}
	
	return 0;
}



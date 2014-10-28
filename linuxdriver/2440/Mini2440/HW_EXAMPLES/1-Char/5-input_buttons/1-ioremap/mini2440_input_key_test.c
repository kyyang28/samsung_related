
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <poll.h>
#include <linux/input.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>

int fd;

void printUsage(char *name)
{
    printf("[USER]%s /dev/event<x>, x could be 0,1,2,etc..\n", name);
}

void signal_function(int sig)
{
	struct input_event event;
	
	while (read(fd, &event, sizeof(event)) != -1)
		printf("type: %d, code = %d, val = %d\n", event.type, event.code, event.value);
}


/* buttons_test [nonblock] */
int main(int argc, char **argv)
{
	long flag;
	int main_cnt = 0;

    if (argc < 2) {
        printUsage(argv[0]);
        return -1;
    }

	fd = open(argv[1], O_RDWR|O_NONBLOCK);

	if (fd < 0) {
		printf("can't open %s\n", argv[1]);
		return -1;
	}

	signal(SIGIO, signal_function);

	/*  */
	flag = fcntl(fd, F_GETFL);
	flag |= FASYNC;
	fcntl(fd, F_SETFL, flag);

	/* tel driver: the app's pid  */
	fcntl(fd, F_SETOWN, getpid());

	while (1) {
		printf("main %d\n", main_cnt++);
		sleep(10);			
	}

	return 0;
}


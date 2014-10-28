
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

void signal_function(int sig)
{
	struct input_event event;
	
	while (read(fd, &event, sizeof(event)) != -1)
	{
		printf("type: %d, code = %d, val = %d\n", event.type, event.code, event.value);
	}
}


/* buttons_test [nonblock] */
int main(int argc, char **argv)
{
	char buf[1];
	int len;
	int i;
	int ret;
	int cnt = 0;
	int app_cnt = 0;
	long flag;
	int main_cnt = 0;

	fd = open("/dev/input/event1", O_RDWR|O_NONBLOCK);

	if (fd < 0)
	{
		printf("can't open /dev/input/event1\n");
		return -1;
	}

	signal(SIGIO, signal_function);

	/*  */
	flag = fcntl(fd, F_GETFL);
	flag |= FASYNC;
	fcntl(fd, F_SETFL, flag);

	/* tel driver: the app's pid  */
	fcntl(fd, F_SETOWN, getpid());

	while (1)
	{
		printf("main %d\n", main_cnt++);
		
		sleep(10);			
	}

	return 0;
}


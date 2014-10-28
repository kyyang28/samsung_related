

#include <stdio.h>
#include <linux/watchdog.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>


int fd;

void signal_int(int signum)
{
    printf("[USER]signal_int[ctrl+c]!\n");
	write(fd, "V", 1);
	close(fd);
	exit(-1);
}

void signal_tstp(int signum)
{
    printf("[USER]signal_tstp[ctrl+z]!\n");
	write(fd, "V", 1);
	close(fd);
	exit(-1);
}

int main(int argc, char *argv[])
{
	int reset_time, feeddog_time;

    signal(SIGTSTP, signal_tstp);       /* SIGTSTP is for ctrl+z */
	signal(SIGINT, signal_int);         /* SIGINT is for ctrl+c */

	if ((fd = open(argv[1], O_WRONLY)) == -1) {
		printf("[USER]ERROR: failed to open %s!\n", argv[1]);
		exit(-1);
	}
    
	feeddog_time = atoi(argv[2]);
	
	ioctl(fd, WDIOC_GETTIMEOUT, &reset_time);
	printf("\n[USER]Watchdog timeout value is %d\n", reset_time);
	
	reset_time = atoi(argv[3]);
	ioctl(fd, WDIOC_SETTIMEOUT, &reset_time);

    if (feeddog_time != 0) {
		while (1) {
			write(fd, "A", 1);
			ioctl(fd, WDIOC_KEEPALIVE);
			sleep(feeddog_time);
		}
	}

    close(fd);
	return 0;
}


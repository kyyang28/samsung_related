
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

unsigned char ubuf[6];

void led_on(int fd, int num)
{
	if (num == 1)
		ioctl(fd, 0, num);
	else if (num == 2)
		ioctl(fd, 0, num);
	else if (num == 3)
		ioctl(fd, 0, num);
	else if (num == 4)
		ioctl(fd, 0, num);
}

void led_off(int fd, int num)
{
	if (num == 1)
		ioctl(fd, 1, num);
	else if (num == 2)
		ioctl(fd, 1, num);
	else if (num == 3)
		ioctl(fd, 1, num);
	else if (num == 4)
		ioctl(fd, 1, num);
}

/* 
 *	2nd param = status(1:off; 0:on) 
 *	
 */
void leds_all_off(int fd)
{
	ioctl(fd, 1, 1);	
	ioctl(fd, 1, 2);
	ioctl(fd, 1, 3);
	ioctl(fd, 1, 4);
}

void leds_all_on(int fd)
{
	ioctl(fd, 0, 1);
	ioctl(fd, 0, 2);
	ioctl(fd, 0, 3);
	ioctl(fd, 0, 4);
}

int main(int argc, char **argv)
{
	int fd_buttons, fd_leds;

	memset(ubuf, 0, sizeof(ubuf));

	fd_buttons = open("/dev/buttonsPoll", O_RDWR);
	if (fd_buttons < 0) {
		printf("Error: open \"/dev/buttonsPoll\" is failed!\n");
		return -1;
	}

	fd_leds = open("/dev/leds", O_RDWR);
	if (fd_leds < 0) {
		printf("Error: open \"/dev/leds\" is failed!\n");
		return -1;
	}

	leds_all_off(fd_leds);

	/* Read buttons info by using polling method */
	while (1) {
		read(fd_buttons, ubuf, sizeof(ubuf));

		if (!ubuf[0]) {
			printf("key1 is pressed down!\n");
			led_on(fd_leds, 1);
		}else if (!ubuf[1]) {
			printf("key2 is pressed down!\n");
			led_on(fd_leds, 2);
		}else if (!ubuf[2]) {
			printf("key3 is pressed down!\n");
			led_on(fd_leds, 3);
		}else if (!ubuf[3]) {
			printf("key4 is pressed down!\n");
			led_on(fd_leds, 4);
		}else if (!ubuf[4]) {
			printf("key5 is pressed down!\n");
			leds_all_off(fd_leds);
		}else if (!ubuf[5]) {
			printf("key6 is pressed down!\n");
			leds_all_on(fd_leds);
		}
	}

	return 0;
}


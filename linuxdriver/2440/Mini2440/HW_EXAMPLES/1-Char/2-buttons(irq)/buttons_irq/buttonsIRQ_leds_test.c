
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

void leds_on(int fd, int ledNum)
{
	if (ledNum >= 1 && ledNum <= 4)
		ioctl(fd, 0, ledNum);
	else
		printf("ERROR: wrong leds number, please using 1,2,3,4\n");
}

void leds_off(int fd, int ledNum)
{
	if (ledNum >= 1 && ledNum <= 4)
		ioctl(fd, 1, ledNum);
	else
		printf("ERROR: wrong leds number, please using 1,2,3,4\n");
}

void leds_all_on(int fd)
{
	ioctl(fd, 0, 1);
	ioctl(fd, 0, 2);
	ioctl(fd, 0, 3);
	ioctl(fd, 0, 4);
}

void leds_all_off(int fd)
{
	ioctl(fd, 1, 1);
	ioctl(fd, 1, 2);
	ioctl(fd, 1, 3);
	ioctl(fd, 1, 4);
}

int main(int argc, char **argv)
{
	int fd_buttons, fd_leds;
	int buttons_vals[6];

	fd_buttons = open("/dev/buttonsIRQ", O_RDWR);
	if (fd_buttons < 0) {
		printf("ERROR: open \"/dev/buttonsIRQ\" is failed!\n");
		return -1;
	}

	fd_leds = open("/dev/leds", O_RDWR);
	if (fd_leds < 0) {
		printf("ERROR: open \"/dev/leds\" is failed!\n");
		close(fd_buttons);
		return -1;
	}

	leds_all_off(fd_leds);

	while (1) {
		read(fd_buttons, buttons_vals, sizeof(buttons_vals));

		if (buttons_vals[0]) {
			printf("Key1 is pressed down!\n");
			leds_on(fd_leds, 1);
		}else if (buttons_vals[1]) {
			printf("Key2 is pressed down!\n");
			leds_on(fd_leds, 2);
		}else if (buttons_vals[2]) {
			printf("Key3 is pressed down!\n");
			leds_on(fd_leds, 3);
		}else if (buttons_vals[3]) {
			printf("Key4 is pressed down!\n");
			leds_on(fd_leds, 4);
		}else if (buttons_vals[4]) {
			printf("Key5 is pressed down!\n");
			leds_all_on(fd_leds);
		}else if (buttons_vals[5]) {
			printf("Key6 is pressed down!\n");
			leds_all_off(fd_leds);
		}
	}

	return 0;
}



#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>         /* memset */
#include <unistd.h>         /* read */
#include <sys/ioctl.h>

static int leds_on(int fd, char *fileName, int lednum)
{
    if (lednum != 1 && lednum != 2 && lednum != 3 && lednum != 4) {
        printf("ERROR: Wrong leds number, please enter 1, 2, 3,or 4!\n");
        return -1;
    }

    ioctl(fd, 0, lednum);
    return 0;
}

static int leds_off(int fd, char *fileName, int lednum)
{
    if (lednum != 1 && lednum != 2 && lednum != 3 && lednum != 4) {
        printf("ERROR: Wrong leds number, please enter 1, 2, 3,or 4!\n");
        return -1;
    }

    ioctl(fd, 1, lednum);
    return 0;
}

static int leds_all_on(int fd)
{
    ioctl(fd, 0, 1);
    ioctl(fd, 0, 2);
    ioctl(fd, 0, 3);
    ioctl(fd, 0, 4);
    return 0;
}

static int leds_all_off(int fd)
{
    ioctl(fd, 1, 1);
    ioctl(fd, 1, 2);
    ioctl(fd, 1, 3);
    ioctl(fd, 1, 4);
    return 0;
}

int main(int argc, char **argv)
{
    int tiny210_buttonsPoll_fd, tiny210_leds_fd;
    int ubuf[8];

    tiny210_buttonsPoll_fd = open("/dev/tiny210_buttonsPoll", O_RDWR);
    if (tiny210_buttonsPoll_fd < 0) {
        printf("ERROR: Failed to open \"/dev/tiny210_buttonsPoll\"!\n");
        return -1;
    }

    tiny210_leds_fd = open("/dev/tiny210_leds", O_RDWR);
    if (tiny210_leds_fd < 0) {
        printf("ERROR: Failed to open \"/dev/tiny210_leds\"!\n");
        close(tiny210_buttonsPoll_fd);
        return -1;
    }

    memset(ubuf, 0, sizeof(ubuf));

    while (1) {
        read(tiny210_buttonsPoll_fd, ubuf, sizeof(ubuf));

        if (!ubuf[0]) {
            printf("Key 1 is pressed down!\n");
            leds_on(tiny210_leds_fd, argv[0], 1);
        }else if (!ubuf[1]) {
            printf("Key 2 is pressed down!\n");
            leds_on(tiny210_leds_fd, argv[0], 2);
        }else if (!ubuf[2]) {
            printf("Key 3 is pressed down!\n");
            leds_on(tiny210_leds_fd, argv[0], 3);
        }else if (!ubuf[3]) {
            printf("Key 4 is pressed down!\n");
            leds_on(tiny210_leds_fd, argv[0], 4);
        }else if (!ubuf[4]) {
            printf("Key 5 is pressed down!\n");
            leds_on(tiny210_leds_fd, argv[0], 1);
            leds_on(tiny210_leds_fd, argv[0], 3);
        }else if (!ubuf[5]) {
            printf("Key 6 is pressed down!\n");
            leds_on(tiny210_leds_fd, argv[0], 2);
            leds_on(tiny210_leds_fd, argv[0], 4);
        }else if (!ubuf[6]) {
            printf("Key 7 is pressed down!\n");
            leds_all_on(tiny210_leds_fd);
        }else if (!ubuf[7]) {
            printf("Key 8 is pressed down!\n");
            leds_all_off(tiny210_leds_fd);
        }
    }
    
    return 0;
}


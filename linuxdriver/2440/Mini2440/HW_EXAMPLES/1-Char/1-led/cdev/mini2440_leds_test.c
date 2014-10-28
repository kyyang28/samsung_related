
#include <stdio.h>
#include <unistd.h>     //  write
#include <sys/types.h>  //  open
#include <sys/stat.h>   //  open
#include <fcntl.h>      //  open
#include <sys/ioctl.h>
#include <string.h>

#define LED_1                       1
#define LED_2                       2
#define LED_3                       3
#define LED_4                       4
#define LED_ALL                     5

#define LOW_LEVEL                   0
#define HIGH_LEVEL                  1 

void printUsage(char *fileName)
{
    printf("[Usage]: %s <1|2|3|4|5> <on|off>\n", fileName);
}

int main(int argc, char **argv)
{
    int fd;

    unsigned int cmd;
    unsigned long arg;

    if (argc != 3) {
        printUsage(argv[0]);
        return -1;
    }

    fd = open("/dev/mini2440_leds", O_RDWR);
    if (fd < 0) {
        printf("[USER]Error: Open /dev/mini2440_leds is failed!\n");
        return -1;
    }else {
        printf("[USER]Open /dev/mini2440_leds is successful!\n");
    }


    if (!strcmp(argv[1], "1"))
        cmd = LED_1;
    else if (!strcmp(argv[1], "2"))
        cmd = LED_2;
    else if (!strcmp(argv[1], "3"))
        cmd = LED_3;
    else if (!strcmp(argv[1], "4"))
        cmd = LED_4;
    else if (!strcmp(argv[1], "5"))
        cmd = LED_ALL;
    else {
        printf("[USER]Please use <1|2|3|4|5> as the second argument!\n");
        return -1;        
    }

    printf("[USER]cmd = %u\n", cmd);

    if (!strcmp(argv[2], "on")) {
        arg = LOW_LEVEL;
        printf("[USER]arg = %lu\n", arg);
        ioctl(fd, cmd, arg);
    }else if (!strcmp(argv[2], "off")) {
        arg = HIGH_LEVEL;
        printf("[USER]arg = %lu\n", arg);
        ioctl(fd, cmd, arg);
    }else {        
        printf("[USER]Please use <on|off> as the third argument!\n");
        return -1;
    }

    return 0;
}



#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>

static void delay(void)
{
    int i;
    for (i = 0; i < 60000000; i++)
        ;
}

static void printUsage(char *fileName)
{
    printf("Usage: %s <1|2|3|4|all> <on|off>\n", fileName);
}

static int leds_on(int fd, char *fileName, int lednum)
{
    if (lednum != 1 && lednum != 2 && lednum != 3 && lednum != 4) {
        printUsage(fileName);
        return -1;
    }

    ioctl(fd, 0, lednum);
    return 0;
}

static int leds_off(int fd, char *fileName, int lednum)
{
    if (lednum != 1 && lednum != 2 && lednum != 3 && lednum != 4) {
        printUsage(fileName);
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

/* 
 *  ./tiny210_leds_test 1 on
 *  ./tiny210_leds_test 1 off
 *  ./tiny210_leds_test all on
 *  ./tiny210_leds_test all off 
 */
int main(int argc, char **argv) 
{
    int tiny210_leds_fd;
    
    tiny210_leds_fd = open("/dev/tiny210_leds", O_RDWR);
    if (tiny210_leds_fd < 0) {
        printf("ERROR: Failed to open \"/dev/tiny210_leds\"!\n");
        return -1;
    }

    if (argc > 1) {

        unsigned long leds_status;
        unsigned int leds_num;

        if (argc != 3 && argc != 2) {
            printUsage(argv[0]);
            close(tiny210_leds_fd);
            return -1;
        }
        
        if (!strcmp(argv[1], "1"))
            leds_num = 1;
        else if (!strcmp(argv[1], "2"))
            leds_num = 2;
        else if (!strcmp(argv[1], "3"))
            leds_num = 3;
        else if (!strcmp(argv[1], "4"))
            leds_num = 4;
        else if (!strcmp(argv[1], "all"))
            if (!strcmp(argv[2], "on")) {
                leds_all_on(tiny210_leds_fd);
                close(tiny210_leds_fd);
                return 0;   
            }else if (!strcmp(argv[2], "off")) {
                leds_all_off(tiny210_leds_fd);
                close(tiny210_leds_fd);
                return 0;   
            }else {
                printf("ERROR: Wrong led status!\n");
                printUsage(argv[0]);
                close(tiny210_leds_fd);
                return -1;
            }                
        else {
            printf("ERROR: Wrong led number!\n");
            printUsage(argv[0]);
            close(tiny210_leds_fd);
            return -1;
        }
        
        if (!strcmp(argv[2], "on"))
            leds_status = 0;        
        else if (!strcmp(argv[2], "off"))
            leds_status = 1;
        else {
            printf("ERROR: Wrong led status!\n");
            printUsage(argv[0]);
            close(tiny210_leds_fd);
            return -1;
        }
        
        ioctl(tiny210_leds_fd, leds_status, leds_num);

    }else {        
        while (1) {
            leds_on(tiny210_leds_fd, argv[0], 1);
            delay();
            leds_off(tiny210_leds_fd, argv[0], 1);
            delay();
        }
    }
        
    close(tiny210_leds_fd);
    return 0;
}

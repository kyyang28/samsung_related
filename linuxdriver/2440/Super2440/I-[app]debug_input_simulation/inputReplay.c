
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#define INPUT_REPLAY                (0)
#define INPUT_TAG                   (1)

/*
 *  Usage
 *  
 *  ./inputReplay write <fileName>
 *  ./inputReplay replay
 *  ./inputReplay tag <string>
 */
 
void printUsage(char *execName)
{
    printf("Usage: \n");
    printf("%s write <fileName>\n", execName);
    printf("%s replay\n", execName);
    printf("%s tag <string>", execName);
}

int main(int argc, char **argv)
{
    int fd;
    int fd_data;
    int buf[100];
    int count;
    
    if ((argc != 2) && (argc != 3)) {
        printUsage(argv[0]);
        return -1;
    }   

    fd = open("/dev/inputReplay", O_RDWR);
    if (fd < 0) {
        printf("InputReplay[Charles.Y]: Cannot open /dev/inputReplay\n");
        return -1;
    }

    if (strcmp(argv[1], "replay") == 0) {
        ioctl(fd, INPUT_REPLAY);
    } else if (strcmp(argv[1], "write") == 0) {
        if (argc != 3) {
            printUsage(argv[0]);
            return -1;            
        }

        fd_data = open(argv[2], O_RDONLY);
        if (fd_data < 0) {
            printf("InputReplay[Charles.Y]: Cannot open %s\n", argv[2]);
            return -1;
        }

        while (1) {
            count = read(fd_data, buf, 100);
            if (count == 0) {
                printf("InputReplay[Charles.Y]: Write reaches the end of file, finished\n");
                break;
            } else {
                write(fd, buf, count); 
            }
        }
    } 
    else if (strcmp(argv[1], "tag") == 0) {
        if (argc != 3) {
            printUsage(argv[0]);
            return -1;            
        }
        ioctl(fd, INPUT_TAG, argv[2]);
    } else {
        printUsage(argv[0]);
        return -1;            
    }
    
    return 0;
}



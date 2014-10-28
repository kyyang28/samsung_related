

/*
 *  Usage
 *  r: read;    w: write
 *
 *  ./regeditor <r8>  <addr> [num]
 *  ./regeditor <r16> <addr> [num]
 *  ./regeditor <r32> <addr> [num]
 *
 *  ./regeditor <w8>  <addr> <val>
 *  ./regeditor <w16> <addr> <val>
 *  ./regeditor <w32> <addr> <val>
 */

 
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>



#define KERNEL_RW_R8            0
#define KERNEL_RW_R16           1
#define KERNEL_RW_R32           2

#define KERNEL_RW_W8            3
#define KERNEL_RW_W16           4
#define KERNEL_RW_W32           5


void print_usage(char *name)
{
    printf("Usage:\n");
    printf("%s <r8 | r16 | r32> <phys_addr> [num]\n", name);
    printf("%s <w8 | w16 | w32> <phys_addr> <val>\n", name);
}

int main(int argc, char **argv)
{
    int fd;
    unsigned int buf[2];
    unsigned int num;
    unsigned int i;
    
    if ((argc != 3) && (argc != 4)) {
        print_usage(argv[0]);
        return -1;
    }

    fd = open("/dev/kernelRW", O_RDWR);
    if (fd < 0) {
        printf("Cannot open kernelRW!\n");
        return -2;
    }
        
    buf[0] = strtoul(argv[2], NULL, 0);

    if (argc == 4) {
        buf[1] = strtoul(argv[3], NULL, 0);
        num = buf[1];    
    } else {
        num = 1;
    }

    if (strcmp(argv[1], "r8") == 0) {
        for (i = 0; i < num; i++) {
            ioctl(fd, KERNEL_RW_R8, buf);
            printf("%02d. [%08x] = %08x\n", i, buf[0], (unsigned char)buf[1]);
            buf[0] += 1;
        }
    } else if (strcmp(argv[1], "r16") == 0) {
        for (i = 0; i < num; i++) {
            ioctl(fd, KERNEL_RW_R16, buf);
            printf("%02d. [%08x] = %08x\n", i, buf[0], (unsigned short)buf[1]);
            buf[0] += 2;
        }
    } else if (strcmp(argv[1], "r32") == 0) {
        printf("Charles.Y[app]: r32\n");
        for (i = 0; i < num; i++) {
            ioctl(fd, KERNEL_RW_R32, buf);
            printf("%02d. [%08x] = %08x\n", i, buf[0], (unsigned int)buf[1]);
            buf[0] += 4;
        }
    } else if (strcmp(argv[1], "w8") == 0) {
        ioctl(fd, KERNEL_RW_W8, buf);
    } else if (strcmp(argv[1], "w16") == 0) {
        ioctl(fd, KERNEL_RW_W16, buf);
    } else if (strcmp(argv[1], "w32") == 0) {
        ioctl(fd, KERNEL_RW_W32, buf);
    } else {
        print_usage(argv[0]);
        return -1;
    }
    
    return 0;
}




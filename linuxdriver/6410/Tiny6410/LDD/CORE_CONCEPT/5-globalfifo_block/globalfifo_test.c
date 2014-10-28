
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>


#define MEM_CLEAR       0x1     /* The code to clear the memory region to zero */

char revtext[100];
char sendtext[] = "Love ARM Linux!";

int main(int argc, char **argv)
{
    int fd;

    fd = open("/dev/globalfifo", O_RDWR);

    if (fd < 0) {
        printf("[USER]Error: can't open /dev/globalfifo\n");
        return -1;
    }

    write(fd, sendtext, sizeof(sendtext));

    memset(revtext, 0, sizeof(revtext));

    lseek(fd, 0, 0);    /* Offset(2nd argument) is 0, whence(3rd argument) = 0(case 0) */    
    read(fd, revtext, sizeof(revtext));
    printf("[USER]Before ioctl clear revtext = %s\n", revtext);

    ioctl(fd, MEM_CLEAR);    
    lseek(fd, 0, 0);    /* Offset(2nd argument) is 0, whence(3rd argument) = 0(case 0) */    
    read(fd, revtext, sizeof(revtext));
    printf("[USER]After ioctl clear revtext = %s\n", revtext);

    return 0;
}


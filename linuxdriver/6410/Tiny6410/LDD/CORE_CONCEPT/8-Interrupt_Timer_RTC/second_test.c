
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc, char **argv)
{
    int fd;
    int cnt = 0;
    int old_cnt = 0;

    fd = open("/dev/second", O_RDWR);
    if (fd < 0) {
        printf("[USER] Could not open /dev/second device node!\n");
        return -1;
    }
    
    while (1) {
        read(fd, &cnt, sizeof(unsigned int));
        if (cnt != old_cnt) {
            printf("[USER] Seconds after open /dev/second: %d\n", cnt);
            old_cnt = cnt;
        }
    }

    close(fd);
    
    return 0;
}



#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/select.h>     /* select */
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>           /* poll */


#define FIFO_CLEAR          0x1     /* The code to clear the FIFO region to zero */

int main(int argc, char **argv)
{
    int fd;
    fd_set rfds, wfds;

    /* open the device node with non-blocking */
    fd = open("/dev/globalfifo", O_RDWR | O_NONBLOCK);

    if (fd < 0) {
        printf("[USER]Error: can't open /dev/globalfifo\n");
        return -1;
    }

    /* Clearing FIFO to zero */
    if (ioctl(fd, FIFO_CLEAR, 0) < 0) {
        printf("[USER]Error: failed to clear the FIFO using ioctl!\n");
        return -1;
    }
    
    while (1) {
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_SET(fd, &rfds);
        FD_SET(fd, &wfds);

        select(fd+1, &rfds, &wfds, NULL, NULL);

        /* Data can be read */
        if (FD_ISSET(fd, &rfds))
            printf("[USER]: Data can be read!\n");

        /* Data can be write */
        if (FD_ISSET(fd, &wfds))
            printf("[USER]: Data can be written!\n");
    }
    
    return 0;
}


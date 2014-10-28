
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>


void input_handler(int signum)
{
    printf("receive a signal from globalfifo, signalnum:%d\n", signum);
}

int main(int argc, char **argv)
{
    int fd, flags;

    fd = open("/dev/globalfifo", O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        printf("[USER]Open /dev/globalfifo failed!\n");
        return -1;
    }

    signal(SIGIO, input_handler);
    fcntl(fd, F_SETOWN, getpid());
    flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | FASYNC);

    while (1)
        sleep(100);
    
    return 0;
}


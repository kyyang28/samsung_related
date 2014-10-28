
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



void print_usage(char *file)
{
    printf("%s <dev>\n", file);
}

int main(int argc, char **argv)
{
    int fd;

    if (argc != 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    fd = open(argv[1], O_RDWR);

    if (fd < 0) 
        printf("Cannot open %s!\n", argv[1]);
    else 
        printf("Can open %s!\n", argv[1]);
    
    return 0;
}



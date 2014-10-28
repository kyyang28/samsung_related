
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

//#define MINI2440_DBG            printf
#define MINI2440_DBG(...)

int main(int argc, char **argv)
{
	int fd;
	int cnt = 0;
    unsigned char key_vals[6];

	fd = open("/dev/mini2440_buttons", O_RDWR);

	if(fd < 0) {
		printf("[APP]/dev/mini2440_buttons cannot be opened!\n");
        return -1;
    }else {
        printf("\n\n[APP]/dev/mini2440_buttons is opened successfully!\n\n");
    }
    
	while(1) {
		read(fd, key_vals, sizeof(key_vals));
        MINI2440_DBG("\n[APP]%c %c %c %c %c %c", key_vals[0], key_vals[1], key_vals[2], key_vals[3], 
                    key_vals[4], key_vals[5]);
		if(key_vals[0] || key_vals[1] || key_vals[2] || key_vals[3] || 
           key_vals[4] || key_vals[5]) {
			printf("[APP]cnt:%d, key pressed: %c %c %c %c %c %c\n",
				   cnt++, key_vals[0], key_vals[1], key_vals[2], key_vals[3], 
				   key_vals[4], key_vals[5]);
		}
	}
    
	return 0;
}



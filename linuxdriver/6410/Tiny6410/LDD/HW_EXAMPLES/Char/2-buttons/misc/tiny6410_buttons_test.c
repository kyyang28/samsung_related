
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

//#define TINY6410_DBG            printf
#define TINY6410_DBG(...)

int main(int argc, char **argv)
{
	int fd;
	int cnt = 0;
    unsigned char key_vals[8];

	fd = open("/dev/tiny6410_buttons", O_RDWR);

	if(fd < 0) {
		printf("[APP]/dev/tiny6410_buttons cannot be opened!\n");
        return -1;
    }else {
        printf("\n\n[APP]/dev/tiny6410_buttons is opened successfully!\n\n");
    }
    
	while(1) {
		read(fd, key_vals, sizeof(key_vals));
        TINY6410_DBG("\n[APP]%c %c %c %c %c %c %c %c", key_vals[0], key_vals[1], key_vals[2], key_vals[3], 
                    key_vals[4], key_vals[5], key_vals[6], key_vals[7]);
		if(key_vals[0] || key_vals[1] || key_vals[2] || key_vals[3] || 
           key_vals[4] || key_vals[5] || key_vals[6] || key_vals[7]) {
			printf("[APP]cnt:%d, key pressed: %c %c %c %c %c %c %c %c\n",
				   cnt++, key_vals[0], key_vals[1], key_vals[2], key_vals[3], 
				   key_vals[4], key_vals[5], key_vals[6], key_vals[7]);
		}
	}
    
	return 0;
}



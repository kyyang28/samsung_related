
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>


int main(int argc, char **argv)
{
	int fd;
	int cnt = 0;
	unsigned char key_vals[6];

	fd = open("/dev/button_poll", O_RDWR);

	if(fd < 0) 
		printf("cannot open!\n");
	
	while(1) {
		read(fd, key_vals, sizeof(key_vals));
		if(!key_vals[0] || !key_vals[1] || !key_vals[2] || !key_vals[3] || !key_vals[4] || !key_vals[5]) {
			printf("%06d key pressed: %d %d %d %d %d %d\n",
				   cnt++, key_vals[0], key_vals[1], key_vals[2], key_vals[3], key_vals[4], key_vals[5]);
		}
	}
	return 0;
}



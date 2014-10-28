/*****************************************
 Copyright 2001-2003	
 Sigma Designs, Inc. All Rights Reserved
 Proprietary and Confidential
 *****************************************/
 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "fb.h"

 


struct fb_var_screeninfo fb_var;
struct fb_fix_screeninfo fb_fix;
char * fb_base_addr = NULL;

void printusage(char *name)
{
	fprintf(stderr,"Usage (example): %s /dev/fb\n", name);
}

int main(int argc, char** argv)
{
	int fd = 0;
	int screensize = 0;
	unsigned long *p;

	/* 0xrrggbb */
	int colors[] = {0xff0000, 0x00ff00, 0x0000ff, 0x0, 0xffffff, 0xc2a24b};
    int i, j;
	int col = 0;

	
	if (argc < 2) {
        printusage(argv[0]);
		return -1;
	}
 
	fd = open(argv[1],O_RDWR);
	if (fd <0){
		printf("error opening %s\n",argv[1]);
		exit(1);
	}


	/* Get fixed screen information */
	if (ioctl(fd, FBIOGET_FSCREENINFO, &fb_fix)) {
		printf("Error reading fb fixed information.\n");
		exit(1);
	}

	/* Get variable screen information 	*/
	if (ioctl(fd, FBIOGET_VSCREENINFO, &fb_var)) {
		printf("Error reading fb variable information.\n");
		exit(1);
	}

	printf("LCD: %d x %d, %d bpp\n", fb_var.xres, fb_var.yres, fb_var.bits_per_pixel);

	if (fb_var.bits_per_pixel == 24)
		screensize = fb_var.xres * fb_var.yres * 32 / 8;
	else
		screensize = fb_var.xres * fb_var.yres * fb_var.bits_per_pixel / 8;
		
	
	fb_base_addr = (char *)mmap(NULL , screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if ((int)fb_base_addr == -1) {
		printf("error mapping fb\n");
		exit(1);
	}

	while (1)
	{
		p = fb_base_addr;
		for (i = 0; i < fb_var.xres; i++)
		{
			for (j = 0; j < fb_var.yres; j++)
			{
				*p = colors[col];
				p++;
			}
		}

		col++;
		if (col == sizeof(colors)/sizeof(colors[0]))
			col = 0;

		sleep(3);
	}
	

    
	close(fd);
	return 0;
}


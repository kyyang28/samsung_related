
#ifndef __MINI2440_LCD_DRV_H
#define __MINI2440_LCD_DRV_H

#define XRES                    240
#define XRES_VIRTUAL            XRES
#define LEFT_MARGIN             10
#define RIGHT_MARGIN            26
#define HSYNC_LEN               6

#define YRES                    320
#define YRES_VIRTUAL            YRES
#define UPPER_MARGIN            2
#define LOWER_MARGIN            16
#define VSYNC_LEN               2

#define BPP                     16

struct mini2440lcd_info {
    struct fb_info *fbi;
    void __iomem        *lcd_io;
    struct resource     *lcd_mem;
    u32 pseudo_palette[16];
};

#endif


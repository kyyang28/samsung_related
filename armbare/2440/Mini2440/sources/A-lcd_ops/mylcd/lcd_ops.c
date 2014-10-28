

/* LCD GPIO registers */
#define GPCCON					(*(volatile unsigned long *)0x56000020)
#define GPDCON					(*(volatile unsigned long *)0x56000030)
#define GPGCON					(*(volatile unsigned long *)0x56000060)

/* LCD Controller registers */
#define LCDCON1					(*(volatile unsigned long *)0x4D000000)
#define LCDCON2					(*(volatile unsigned long *)0x4D000004)
#define LCDCON3					(*(volatile unsigned long *)0x4D000008)
#define LCDCON4					(*(volatile unsigned long *)0x4D00000C)
#define LCDCON5					(*(volatile unsigned long *)0x4D000010)
#define LCDSADDR1				(*(volatile unsigned long *)0x4D000014)
#define LCDSADDR2				(*(volatile unsigned long *)0x4D000018)
#define LCDSADDR3				(*(volatile unsigned long *)0x4D00001C)
#define TPAL					(*(volatile unsigned long *)0x4D000050)

/* LCD related parameters */
#define LCD_XRES				(240)
#define LCD_YRES				(320)
#define BPP						(16)

#define LCD_FB_BASE_ADDR		(0x30008000)

/* LCDCON1 VALUES */
#define LCDCON1_CLKVAL			(0x06)
#define LCDCON1_PNRMODE			(0x03)
#define LCDCON1_BPPMODE			(0x0C)
#define LCDCON1_ENVID_ENABLE	(0x01)
#define LCDCON1_ENVID_DISABLE	(0x00)
#define LCDCON1_VALS			((LCDCON1_CLKVAL<<8) | (LCDCON1_PNRMODE<<5) | \
								(LCDCON1_BPPMODE<<1) | (LCDCON1_ENVID_DISABLE))

/* LCDCON2 VALUES */
#define UPPER_MARGIN			(2)
#define LOWER_MARGIN			(16)
#define VSYNC_LEN				(2)
#define LCDCON2_VBPD			(UPPER_MARGIN - 1)
#define LCDCON2_LINEVAL			(LCD_YRES - 1)
#define LCDCON2_VFPD			(LOWER_MARGIN - 1)
#define LCDCON2_VSPW			(VSYNC_LEN - 1)
#define LCDCON2_VALS			((LCDCON2_VBPD<<24) | (LCDCON2_LINEVAL<<14) | \
								 (LCDCON2_VFPD<<6) | (LCDCON2_VSPW))

/* LCDCON3 VALUES */
#define LEFT_MARGIN				(10)
#define RIGHT_MARGIN			(18)
#define LCDCON3_HBPD			(RIGHT_MARGIN - 1)
#define LCDCON3_HFPD			(LEFT_MARGIN - 1)
#define LCDCON3_HOZVAL			(LCD_XRES - 1)
#define LCDCON3_VALS			((LCDCON3_HBPD<<19) | (LCDCON3_HOZVAL<<8) | (LCDCON3_HFPD))

/* LCDCON4 VALUES */
#define HSYNC_LEN				(6)
#define LCDCON4_HSPW			(HSYNC_LEN - 1)
#define LCDCON4_VALS			(LCDCON4_HSPW)

/* LCDCON5 VALUES */
#define LCDCON5_BPP24BL			(0x00)	/* the order of 24 bpp video memory - LSB valid */
#define	LCDCON5_FRM565			(0x01)	/* 5:6:5 */
#define LCDCON5_INVVCLK			(0x01)	/* fetch data at rising edge */
#define LCDCON5_INVVLINE		(0x01)	/* VLINE/HSYNC pulse polarity - Inverted */
#define LCDCON5_INVVFRAME		(0x01)	/* VFRAME/VSYNC pulse polarity - Inverted */
#define LCDCON5_INVVDEN			(0x01)	/* VDEN signal polarity - Inverted */
#define LCDCON5_BSWP			(0x00)	/* Byte swap control bit - Swap Disable */
#define LCDCON5_HWSWP			(0x01)	/* Half-Word swap control bit - Swap Enable */
#define LCDCON5_VALS			((LCDCON5_BPP24BL<<12) | (LCDCON5_FRM565<<11) | (LCDCON5_INVVCLK<<10) | \
								 (LCDCON5_INVVLINE<<9) | (LCDCON5_INVVFRAME<<8) | (LCDCON5_INVVDEN<<6) | \
								 (LCDCON5_BSWP<<1) | (LCDCON5_HWSWP))

/* LCDSADDR1 VALUES */
#define LCDSADDR1_VALS		((LCD_FB_BASE_ADDR >> 1) & ~(0x3<<30))

/* LCDSADDR2 VALUES */
#define LCDSADDR2_VALS		((((LCD_FB_BASE_ADDR) + ((LCD_XRES)*(LCD_YRES) * BPP / 8)) >> 1) & 0x1fffff)

/* LCDSADDR3 VALUES */
#define LCDSADDR3_PAGEWIDTH		((LCD_XRES) * BPP / 8 / 2)
#define LCDSADDR3_VALS			(LCDSADDR3_PAGEWIDTH & 0x3ff)


static void lcd_gpio_init(void)
{
	/* Setup the GPCCON and GPDCON registers */
	GPCCON = 0xaaaaaaaa;
	GPDCON = 0xaaaaaaaa;
}

static void lcd_controller_init(void)
{
	/* Setup the LCDCONx registers */
	LCDCON1 = LCDCON1_VALS; 
	LCDCON2 = LCDCON2_VALS; 
	LCDCON3 = LCDCON3_VALS;
	LCDCON4 = LCDCON4_VALS;
	LCDCON5 = LCDCON5_VALS;
	
	/* Setup the LCDSADDRx registers */
	LCDSADDR1 = LCDSADDR1_VALS; 
	LCDSADDR2 = LCDSADDR2_VALS; 
	LCDSADDR3 = LCDSADDR3_VALS;
	
	/* Disable temporary palette register */
	TPAL = 0;
}

static void lcd_enable(void)
{
	GPGCON |= (0x03<<8);	/* LCD_PWREN */
	LCDCON5 |= (0<<5);		/* LCD_PWREN normal */
	LCDCON5 |= (1<<3);		/* Enable PWREN signal */
	LCDCON1 |= (1<<0);		/* Enable the video output and the LCD control signal */
}

void lcd_init(void)
{
	/* Setup the lcd gpio registers */
	lcd_gpio_init();

	/* Setup the lcd controller registers */
	lcd_controller_init();

	/* Enable the lcd signals */
	lcd_enable();
}


/* ############################# Start of the LCD Operations ############################# */
static void lcd_draw_pixel(unsigned int row, unsigned int col, unsigned int color)
{
	unsigned char r, g, b;
	unsigned short *pixel = (unsigned short *)LCD_FB_BASE_ADDR + (row + col * LCD_XRES);
	
	r	= (color >> 19) & 0x1f;		/* 5-bit */
	g	= (color >> 10) & 0x3f;		/* 6-bit */
	b	= (color >> 3)	& 0x1f;		/* 5-bit */
	color = (r << 11) | (g << 5) | (b);
	
	*pixel = (unsigned short)color;
}

void lcd_clear_screen(unsigned int color)
{
	unsigned int i, j;

	for (i = 0; i < LCD_XRES; i++) {
		for (j = 0; j < LCD_YRES; j++) {
			lcd_draw_pixel(i, j, color);
		}
	}
}

void lcd_display_bmp(unsigned int bmp_addr)
{
	unsigned int i, j;
	unsigned char *bmp = (unsigned char *)bmp_addr;
	unsigned char r, g, b;
	unsigned int color;

	bmp += 54;

	for (i = 0; i < LCD_XRES; i++) {
		for (j = 0; j < LCD_YRES; j++) {
			r = *bmp++;
			g = *bmp++;
			b = *bmp++;
			color = (r << 11) | (g << 5) | (b);
			
			lcd_draw_pixel(i, j, color);
		}
	}
}
/* ############################# End of the LCD Operations ############################# */


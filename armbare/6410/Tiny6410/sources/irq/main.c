
#include "uart.h"
#include <types.h>

char *str = "hello uplooking\n\r";

void hello(void)
{
	printf("%s", str);
}

int print_cpsr(unsigned int cpsr, char *why)
{	
	printf("print_cpsr %s cpsr = 0x%x\n\r", why, cpsr);
	return 0;
}


void nand_read_test(void)
{
	int i;
	char buf[100];
	unsigned long addr;
	unsigned long size;
	
	printf("enter the start address: ");
	scanf("%s", buf);
	addr = simple_strtoul(buf, NULL, 0);
	printf("read addr = 0x%x\n\r", addr);

	printf("enter the size: ");
	scanf("%s", buf);
	size = simple_strtoul(buf, NULL, 0);

	if (size > 100)
	{
		printf("the max size is 100\n\r");
		size = 100;
	}

	nand_read(addr, buf, size);

	printf("datas: \n\r");
	for (i = 0; i < size; i++)
	{
		printf("%02x ", buf[i]);
		if ((i+1) % 16 == 0)
		{
			printf("\n\r");
		}
	}
	printf("\n\r");
}

void nand_erase_test(void)
{
	char buf[100];
	unsigned long addr;
	
	printf("enter the start address: ");
	scanf("%s", buf);
	addr = simple_strtoul(buf, NULL, 0);
	printf("erase addr = 0x%x\n\r", addr);

	nand_erase_block(addr);
	
}

void nand_write_test(void)
{
	char buf[100];
	unsigned long addr;
	unsigned long size;
	
	printf("enter the start address: ");
	scanf("%s", buf);
	addr = simple_strtoul(buf, NULL, 0);

	printf("enter the string: ");
	scanf("%s", buf);

	size = strlen(buf) + 1;

	nand_write(addr, buf, size);
	
}


void update_program(void)
{
	unsigned char *buf = (unsigned char *)0x52000000;
	unsigned long len = 0;
	int have_begin = 0;
	int nodata_time = 0;
	char c;

	/* 读串口获得数据 */
	printf("\n\ruse gtkterm to send file\n\r", len);
	while (1)
	{
		if (getc_nowait(&buf[len]) == 0)
		{
			have_begin = 1;
			nodata_time = 0;
			len++;
		}
		else
		{
			if (have_begin)
			{
				nodata_time++;
			}			
		}

		if (nodata_time == 1000)
		{
			break;
		}
	}
	printf("have get %d bytes data\n\r", len);

	printf("Press Y to program the flash: \n\r");

	c = getc();
	
	if (c == 'y' || c == 'Y')
	{	
		/* 烧写到nand flash block 0 */
		nand_erase_block(0);
		nand_write(0, buf, len);
		
		printf("update program successful\n\r");
	}
	else
	{
		printf("Cancel program!\n\r");
	}
}

void run_program(void)
{
	unsigned char *buf = (unsigned char *)0x52000000;
	unsigned long len = 0;
	int have_begin = 0;
	int nodata_time = 0;
	void (*theProgram)(void);

	/* 读串口获得数据 */
	printf("\n\ruse gtkterm to send file\n\r", len);
	while (1)
	{
		if (getc_nowait(&buf[len]) == 0)
		{
			have_begin = 1;
			nodata_time = 0;
			len++;
		}
		else
		{
			if (have_begin)
			{
				nodata_time++;
			}			
		}

		if (nodata_time == 1000)
		{
			break;
		}
	}
	printf("have get %d bytes data\n\r", len);

	printf("jump to 0x52000000 to run it\n\r");
	
	theProgram = (void (*)(void))0x52000000;

	theProgram();		
}



int main()
{
	char c;
	
	init_uart();
	printf("can update program with serial port\n\r");

	while (1)
	{
		printf("[w] write the nand flash\n\r");
		printf("[r] read the nand flash\n\r");
		printf("[e] erase the nand flash\n\r");
		printf("[g] get file, and write to nand flash 0 block\n\r");
		printf("[x] get file to ddr(0x52000000), run it\n\r");

		do {
			c = getc();
			if (c == '\n' || c == '\r')
			{
				printf("\n\r");
			}
			else
			{
				putc(c);
			}
		} while (c == '\n' || c == '\r');
		
		switch (c)
		{
			case 'w':
			case 'W':
			{
				nand_write_test();
				break;
			}

			case 'r':
			case 'R':
			{
				nand_read_test();
				break;
			}

			case 'e':
			case 'E':
			{
				nand_erase_test();
				break;
			}

			case 'g':
			case 'G':
			{
				update_program();
				break;
			}

			case 'x':
			case 'X':
			{
				run_program();
				break;
			}
			
		}
	}
	
	return 0;
}


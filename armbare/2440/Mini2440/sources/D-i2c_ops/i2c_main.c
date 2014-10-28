
#include <stdio.h>
#include "s3c2440-regs.h"
#include "uart_ops.h"
#include "i2c_ops.h"
#include "eeprom_ops.h"
#include "i2c_irq.h"

void i2c_eeprom_test(void)
{
	int i, addr, value;
	char choice;
	char buf[200];

	while (1) {
		printf("\r\n###### i2c_eeprom testing program ######\r\n");
		printf("[r] Read eeprom\r\n");
		printf("[w] Write eeprom\r\n");
		printf("Please enter your selection(r or w): ");

		choice = serial_getc_ll();
		printf("%c\r\n", choice);

		switch (choice) {
			case 'r':
			case 'R':
				printf("Please enter the address: ");
				i = 0;
				do {
					choice = serial_getc_ll();
					buf[i++] = choice;
					serial_putc_ll(choice);
				}while (choice != '\r' && choice != '\n');
				buf[i] = '\0';
				printf("\r\n");

				while (--i >= 0) {
					if (buf[i] < '0' || buf[i] > '9')
						buf[i] = ' ';
				}

				sscanf(buf, "%d", &addr);
				printf("I2c read address is: %d\r\n", addr);
				value = eeprom_read_ll(addr); 
				printf("Read value is: %d\r\n", value);
				break;

			case 'w':
			case 'W':
				printf("Please enter the address: ");
				i = 0;
				do {
					choice = serial_getc_ll();
					buf[i++] = choice;
					serial_putc_ll(choice);
				}while (choice != '\r' && choice != '\n');
				buf[i] = '\0';
				printf("\r\n");

				while (--i >= 0) {
					if (buf[i] < '0' || buf[i] > '9')
						buf[i] = ' ';
				}

				sscanf(buf, "%d", &addr);

				printf("Please enter the value: ");
				i = 0;
				do {
					choice = serial_getc_ll();
					buf[i++] = choice;
					serial_putc_ll(choice);
				}while (choice != '\r' && choice != '\n');
				buf[i] = '\0';
				printf("\r\n");
				
				while (--i >= 0) {
					if (buf[i] < '0' || buf[i] > '9')
						buf[i] = ' ';
				}
				
				sscanf(buf, "%d", &value);
				printf("I2c write address and value are: %d, %d\r\n", addr, value);

				eeprom_write_ll(addr, value);
				break;

			default:
				printf("\r\nWrong selection, please enter 'r','R','w' or 'W'!\r\n");
				break;
		}
	}
}

int i2c_main(void)
{
	/* Uart initialization */
	uart_init_ll();
	
	/* I2c initialization */
	i2c_init_ll();
	
	/* I2c irq initialization */
	i2c_int_init_ll();
	
	/* I2c eeprom operations */
	i2c_eeprom_test();

	return 0;
}



#include <stdio.h>

#define COM0                  (0)

/* UART function */
extern unsigned char uart_getchar(int COM);

/* EEPROM function */
extern void eeprom_write(unsigned char address, unsigned char data);
extern unsigned char eeprom_read(unsigned char address);

int main(void)
{
    char ch;
    char str[200];
    int i, address, data;
    
    while (1)
    {
        printf("\r\n@@@@@@ IIC_EEPROM Testing Program @@@@@@\r\n");
        printf("[R] Read EEPROM\n\r");
        printf("[W] Write EEPROM\n\r");
        printf("Enter your selection(r or w): ");

	ch = uart_getchar(COM0);
        printf("%c\r\n", ch);

        switch (ch) {
            case 'r':
            case 'R':
            {
                printf("Enter address: ");
                i = 0;
                do {
                    ch = uart_getchar(COM0);
                    str[i++] = ch;
                    uart_putchar(COM0, ch);
                } while(ch != '\n' && ch != '\r');
                str[i] = '\0';

                while(--i >= 0) {
                    if (str[i] < '0' || str[i] > '9')
                        str[i] = ' ';
                }

                sscanf(str, "%d", &address);
                printf("\r\nread address = %d\r\n", address);
                data = eeprom_read(address);
                printf("data = %d\r\n", data);
                    
                break;
            }
            
            case 'w':
            case 'W':
            {
                printf("Enter address: ");
                i = 0;
                do {
                    ch = uart_getchar(COM0);
                    str[i++] = ch;
                    uart_putchar(COM0, ch);
                } while(ch != '\n' && ch != '\r');
                str[i] = '\0';
                printf("\r\n");
                
                while(--i >= 0) {
                    if (str[i] < '0' || str[i] > '9')
                        str[i] = ' ';
                }
                
                sscanf(str, "%d", &address);
                //printf("get str %s\r\n", str);

                printf("Enter data: ");
                i = 0;
                do {
                    ch = uart_getchar(COM0);
                    str[i++] = ch;
                    uart_putchar(COM0, ch);
                } while(ch != '\n' && ch != '\r');
                str[i] = '\0';
                printf("\r\n");
                //printf("get str %s\r\n", str);
                
                while(--i >= 0) {
                    if (str[i] < '0' || str[i] > '9')
                        str[i] = ' ';
                }
                
                sscanf(str, "%d", &data);
                printf("write address %d with data %d\r\n", address, data);
			    
                eeprom_write(address, data);
                
                break;
            }
        }
        
    }

    return 0;
}



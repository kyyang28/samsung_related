
#include "string.h"

extern void serial_putc(unsigned char c);
extern unsigned char serial_getc(void);
extern void serial_puts(char *s);

char * get_string(char *s) 
{
	char ch;
	char *buf = s;
	while( (ch = serial_getc()) != '\r' ) {
		if(ch == '\b') {
    		if(buf > s) {		// the num of backspaces should not be exceeded the buffer size 
				buf--;			// delete the character of backspace from the buffer
				*buf = '\0'; // set the last pos be NULL('\0')
                serial_putc('\b');   // backspace
				serial_putc(' ');    // clear the char to empty will move the cursor to the next position
				serial_putc('\b');   // so, we need to backspace again
            }
		}else {
			*buf++ = ch;
			serial_putc(ch);
		}
	}
	*buf = '\0';

    serial_puts("\r\n");
	return s;
}

int put_string(const char *s) 
{
	char ch;
	const char *buf = s;
	while( (ch = *buf++) != '\0') {
		if(ch == '\n') 
			serial_putc('\r');
		serial_putc(ch);
	} 
	return buf - s;
}

void puthex(unsigned char ch) 
{
	static char hex[17] = "0123456789abcdef";
	serial_putc(hex[ch]);
}

void putcharx(unsigned char ch) 
{
	unsigned char c0, c1;
	c0 = ch & 0x0F;
	c1 = (ch & 0xF0) >> 4;
	puthex(c1);
	puthex(c0);
}

void putx(unsigned long value) 
{
	unsigned char c0, c1, c2, c3;
	c0 = value & 0xFF;
	c1 = (value & 0xFF00) >> 8;
	c2 = (value & 0xFF0000) >> 16;
	c3 = (value & 0xFF000000) >> 24;
	putcharx(c3);
	putcharx(c2);
	putcharx(c1);
	putcharx(c0);
}

void put_int(int num, int base) {
	char temp, buf[100];
	int div = num, cnt = 0, pos, len;
		
	static char hex[17] = "0123456789ABCDEF";
	
	// converting int to array
	do {
    	pos	= div % base;
    	div	= div / base;
    	buf[cnt++] = hex[pos];		
	}while(div);
	
	buf[cnt] = '\0';
	
	// swap
	len = strlen(buf);
	for(cnt = 0; cnt < len/2; ++cnt) {
    	temp = buf[cnt];
    	buf[cnt] = buf[len-cnt-1];
    	buf[len-cnt-1] = temp;
	}
	
	put_string(buf);
}

int atoi(char *s) 
{
	int i;
	int c_value;
	int value = 0;
	char ch;
	
	// deal with base 16
	if(s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
		i = 2;
		while( (ch = s[i++]) != '\0' ) {
			c_value = (ch >= '0' && ch <= '9' ? ch - '0' : ch - 'a' + 10);
			value = value * 16 + c_value; 
		}
		return value; 
	}
	
	// deal with base 10
	i = 0;
	while( (ch = s[i++]) != '\0' ) {
		c_value = ch - '0';
		value = value * 10 + c_value;
	}

    return value;
}


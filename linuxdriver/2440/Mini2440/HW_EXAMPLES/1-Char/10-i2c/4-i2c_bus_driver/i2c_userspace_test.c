
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "i2c-dev.h"


void printUsage(char *fileName)
{
    printf("[USER]%s </dev/i2c/0> <devAddr> read addr\n", fileName);
    printf("[USER]%s </dev/i2c/0> <devAddr> write addr dataValue\n", fileName);
}

int main(int argc, char **argv)
{
    int fd;
    unsigned char addr, dataValue;
    int devAddr;

    if ((argc != 5) && (argc != 6)) {
        printUsage(argv[0]);
        return -1;
    }

    if ((fd = open(argv[1], O_RDWR)) < 0) {
        printf("\n[USER]Cannot open %s\n\n", argv[1]);
        return -1;
    }

    devAddr = strtoul(argv[2], NULL, 0);
    
    if (ioctl(fd, I2C_SLAVE, devAddr) < 0) {
        /* ERROR HANDLING; you can check errno to see what went wrong */
        printf("\n[USER]Failed to set devAddr(%d)\n\n", devAddr);
        return -1;
    }

    if (strcmp(argv[3], "read") == 0) {
        addr = strtoul(argv[4], NULL, 0);
        dataValue = i2c_smbus_read_byte_data(fd, addr);
        printf("\n[USER]dataValue is char[%c], int[%d], hex[0x%2x]\n\n", dataValue, dataValue, dataValue);
    }else if ((strcmp(argv[3], "write") == 0) && (argc == 6)) {
        addr = strtoul(argv[4], NULL, 0);
        dataValue = strtoul(argv[5], NULL, 0);
        i2c_smbus_write_byte_data(fd, addr, dataValue);
    }else {
        printUsage(argv[0]);
        return -1;
    }
    
    return 0;
}


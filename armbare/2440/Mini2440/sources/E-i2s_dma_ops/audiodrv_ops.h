#ifndef __AUDIODRV_OPS_H
#define __AUDIODRV_OPS_H

typedef	union {
	unsigned int data;
	unsigned char buf[4];
} data_size_t;

void play_wavfile(int address);
void record_wavdata(int addr, int size);

#endif


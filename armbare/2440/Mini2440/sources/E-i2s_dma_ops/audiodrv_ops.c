

#include "s3c2440-regs.h"
#include "audiodrv_ops.h"
#include "i2s_ops.h"

/* WAV file start address */
void play_wavfile(int address)
{	
	int data_size;
	data_size_t tmp;
	short * pdata;
	
	// 2 choice: 0x10 and 0x12
	int fmt_chunk_size = *(int *)(address + 0x10);
	
	// 2 choice: 0x28 and 0x2A
	int data_chunk_size_offset; 
	data_chunk_size_offset = address + 0x18 + fmt_chunk_size;
	
	/* wav data size */
	tmp.buf[0] = *(unsigned char *)(data_chunk_size_offset);
	tmp.buf[1] = *(unsigned char *)(data_chunk_size_offset+1);
	tmp.buf[2] = *(unsigned char *)(data_chunk_size_offset+2);
	tmp.buf[3] = *(unsigned char *)(data_chunk_size_offset+3);
	data_size = tmp.data;
	
	// get a pointer to wav data
	pdata = (short *)(data_chunk_size_offset + 4);
	
#ifdef DMA_ENABLED
	//init IIS = DMA mode
	//Transmit FIFO access mode select [15] 0 = Normal 1 = DMA
	IISFCON |= 1<<15;
	//init dma mem->IISFIFO
	//Transmit DMA service request [5] 0 = Disable 1 = Enable
	IISCON |= 1<<5;
	
	//start dma
	DISRC2 = (int)pdata;
	DISRCC2 = 0x0;
	DIDST2 = (int)0x55000010;
	DIDSTC2 = 1<<1 | 1<<0;
	DCON2 = (unsigned int)data_size/2;
	DCON2 |= 1<<23 | 1<<20;
	DMASKTRIG2 = 1<<1;
#else
	// IIS function	
	i2s_write_ll(pdata, data_size);
#endif
}  

void record_wavdata(int addr, int size)
{
	// IIS function	
	i2s_read_ll((short *)addr, size);
	
	return;
}



#include <stdio.h>
#include "s3c2440-regs.h"
#include "sd_irq.h"
#include "lib_ops.h"

#define INICLK	300000
#define SDCLK	24000000	//PCLK=49.392MHz
#define MMCCLK	15000000	//PCLK=49.392MHz

#define INT	0

unsigned int *Tx_buffer;	//128[word]*16[blk]=8192[byte]
unsigned int *Rx_buffer;	//128[word]*16[blk]=8192[byte]
volatile unsigned int rd_cnt;
volatile unsigned int wt_cnt;
volatile unsigned int block;
volatile unsigned int TR_end=0;

int Wide=0; // 0:1bit, 1:4bit
int MMC=0;  // 0:SD  , 1:MMC

int  Maker_ID;
char Product_Name[7]; 
int  Serial_Num;

volatile int RCA;

int Chk_BUSYend(void)
{
	int finish;

	finish=SDIDSTA;

	while( !( ((finish&0x08)==0x08) | ((finish&0x20)==0x20) ))
		finish=SDIDSTA;

	if( (finish&0xfc) != 0x08 )
	{
		printf("\r\nDATA:finish=0x%x\r\n", finish);
		SDIDSTA=0xf4;  //clear error state
		return 0;
	}
	return 1;
}

int Chk_DATend(void)
{
	int finish;

	finish=SDIDSTA;

	while( !( ((finish&0x10)==0x10) | ((finish&0x20)==0x20) ))	
		// Chek timeout or data end
		finish=SDIDSTA;

	if( (finish&0xfc) != 0x10 )
	{
		printf("\r\nDATA:finish=0x%x\r\n", finish);
		SDIDSTA=0xec;  // Clear error state
		return 0;
	}
	return 1;
}

int Chk_CMDend(int cmd, int be_resp)
{
	int finish0;

	if(!be_resp)    // No response
	{
		finish0=SDICSTA;
		while((finish0&0x800)!=0x800)	// Check cmd end
			finish0=SDICSTA;
		
		SDICSTA=finish0;// Clear cmd end state
		return 1;
	}
	else	// With response
	{
		finish0=SDICSTA;
		while( !( ((finish0&0x200)==0x200) | ((finish0&0x400)==0x400) ))    // Check cmd/rsp end
			finish0=SDICSTA;

		if((cmd==1) | (cmd==41))	// CRC no check, CMD9 is a long Resp. command.
		{
			if( (finish0&0xf00) != 0xa00 )  // Check error
			{
				SDICSTA=finish0;   // Clear error state
				
				if(((finish0&0x400)==0x400))
					return 0;	// Timeout error
			}
			SDICSTA=finish0;	// Clear cmd & rsp end state
		}
		else	// CRC check
		{
			if( (finish0&0x1f00) != 0xa00 )	// Check error
			{
				printf("\r\nCMD%d:SDICSTA=0x%x, SDIRSP0=0x%x\r\n", cmd, (u32)SDICSTA, (u32)SDIRSP0);
				SDICSTA=finish0;   // Clear error state

				if(((finish0&0x400)==0x400))
					return 0;	// Timeout error
			}
			SDICSTA=finish0;
		}
		return 1;
	}
}

int CMD9(void)//SEND_CSD
{
	SDICARG = RCA<<16;				// CMD9(RCA,stuff bit)
	SDICCON = (0x1<<10)|(0x1<<9)|(0x1<<8)|0x49;	// long_resp, wait_resp, start, CMD9

	printf("\r\nCSD register :\r\n");
	//-- Check end of CMD9
	if(!Chk_CMDend(9, 1)) 
		return 0;

	printf("\r\nSDIRSP0=0x%x\r\nSDIRSP1=0x%x\r\nSDIRSP2=0x%x\r\nSDIRSP3=0x%x\r\n", \
			     (u32)SDIRSP0, (u32)SDIRSP1, (u32)SDIRSP2, (u32)SDIRSP3);
	return 1;
}

void Flush_Rx_buf(void)
{
    //-- Flushing Rx buffer 
	int i;

	Rx_buffer=(unsigned int *)0x31800000;

	for(i=0;i<2048;i++)	//128[word]*16[blk]=8192[byte]
		*(Rx_buffer+i)=0;

	printf("\r\nEnd Rx buffer flush\r\n");
}

void TR_Buf_new(void)
{
    //-- Tx & Rx Buffer initialize
	int i, j;

	Tx_buffer=(unsigned int *)0x31000000;

	j=0;
	for(i=0;i<2048;i++)	//128[word]*16[blk]=8192[byte]
		*(Tx_buffer+i)=i+j;

	Flush_Rx_buf();
}

void View_Rx_buf(void)
{
    //-- Display Rx buffer 
	int i,error=0;

	Tx_buffer=(unsigned int *)0x31000000;
	Rx_buffer=(unsigned int *)0x31800000;

	printf("\r\nCheck Rx data\r\n");

	for(i=0;i<128*block;i++)
	{
		if(Rx_buffer[i] != Tx_buffer[i])
		{
			printf("\r\nTx/Rx error\r\n"); 
			printf("\r\n%d:Tx-0x%08x, Rx-0x%08x\r\n",i,Tx_buffer[i], Rx_buffer[i]);
			error=1;
			break;
		}
	}

	if(!error)
	{
		printf("\r\nThe Tx_buffer is same to Rx_buffer!\r\n");
		printf("\r\nSD CARD Write and Read test is OK!\r\n");
	}
}

void Wt_Int(void)
{
	SRCPND = INT_SDI;
	INTPND = INT_SDI;

	SDIDAT=*Tx_buffer++;
	wt_cnt++;

	if(wt_cnt==128*block)
	{
		INTMSK |= INT_SDI;
		SDIDAT=*Tx_buffer;
		TR_end=1;
	}
}

void Rd_Int(void)
{
	u32 i,status;

	status=SDIFSTA;
	if( (status&0x200) == 0x200 )	// Check Last interrupt?
	{
		for(i=(status & 0x7f)/4;i>0;i--)
		{
			*Rx_buffer++=SDIDAT;
			rd_cnt++;
		}
		SDIFSTA=SDIFSTA&0x200;	//Clear Rx FIFO Last data Ready, YH 040221
	}
	else if( (status&0x80) == 0x80 )	// Check Half interrupt?
	{
		for(i=0;i<8;i++)
		{
			*Rx_buffer++=SDIDAT;
			rd_cnt++;
		}
	}

	SRCPND = INT_SDI;
	INTPND = INT_SDI;
}

void Wt_Block(void)
{
	u32 mode;
	int status;

	wt_cnt=0;    

	printf("\r\nBlock write test[ Interrupting write ]\r\n");

	mode = 0 ;

	SDIFSTA=SDIFSTA | (1<<16);	//YH 040223 FIFO reset

	if(mode!=2)
		SDIDCON=(2<<22)|(1<<20)|(1<<17)|(Wide<<16)|(1<<14)|(3<<12)|(block<<0);	//YH 040220

	SDICARG=0x0;	    // CMD24/25(addr)

REWTCMD:
	switch(mode)
	{
		case INT:
			isr[INT_SDI_OFT] = Wt_Int;
			INTMSK = ~(INT_SDI);

			SDIIMSK=0x10;  // Tx FIFO half int.

			if(block<2)	    // SINGLE_WRITE
			{
				SDICCON=(0x1<<9)|(0x1<<8)|0x58;    //sht_resp, wait_resp, dat, start, CMD24
				if(!Chk_CMDend(24, 1))	//-- Check end of CMD24
					goto REWTCMD;
			}
			else	    // MULTI_WRITE
			{
				SDICCON=(0x1<<9)|(0x1<<8)|0x59;    //sht_resp, wait_resp, dat, start, CMD25
				if(!Chk_CMDend(25, 1))	//-- Check end of CMD25 
					goto REWTCMD;
			}

			SDICSTA=0xa00;	// Clear cmd_end(with rsp)

			while(!TR_end);

			INTMSK |= (INT_SDI);
			TR_end=0;
			SDIIMSK=0;	// All mask
			break;

		default:
			break;
	}

	//-- Check end of DATA
	if(!Chk_DATend()) 
		printf("\r\ndat error\r\n");

	//YH 040220, Clear Data Transfer mode => no operation, Cleata Data Transfer start
	SDIDCON = SDIDCON & ~(7<<12);	
	SDIDSTA=0x10;	// Clear data Tx/Rx end

	if(block>1)
	{
		//--Stop cmd(CMD12)
REWCMD12:    
		SDIDCON=(1<<18)|(1<<17)|(0<<16)|(1<<14)|(1<<12)|(block<<0);	//YH  040220
		SDICARG=0x0;	    //CMD12(stuff bit)
		SDICCON=(0x1<<9)|(0x1<<8)|0x4c;    //sht_resp, wait_resp, start, CMD12

		//-- Check end of CMD12
		if(!Chk_CMDend(12, 1)) 
			goto REWCMD12;
		SDICSTA=0xa00;	// Clear cmd_end(with rsp)

		//-- Check end of DATA(with busy state)
		if(!Chk_BUSYend()) 
			printf("\r\nerror\r\n");
		SDIDSTA=0x08;	//! Should be cleared by writing '1'.
	}
}

void Rd_Block(void)
{
	u32 mode;
	int status;

	rd_cnt=0;    
	printf("\r\nBlock read test[ Interrupting read ]\r\n");

	mode = 0;

	SDIFSTA=SDIFSTA|(1<<16);	// FIFO reset

	if(mode!=2)
		SDIDCON=(2<<22)|(1<<19)|(1<<17)|(Wide<<16)|(1<<14)|(2<<12)|(block<<0);	//YH 040220

	SDICARG=0x0;	// CMD17/18(addr)
	
RERDCMD:
	switch(mode)
	{
		case INT:
			isr[INT_SDI_OFT] = Rd_Int;
			INTMSK = ~(INT_SDI);
			SDIIMSK=5;	// Last & Rx FIFO half int.

			if(block<2)	// SINGLE_READ
			{
				SDICCON=(0x1<<9)|(0x1<<8)|0x51;    // sht_resp, wait_resp, dat, start, CMD17
				if(!Chk_CMDend(17, 1))	//-- Check end of CMD17
					goto RERDCMD;	    
			}
			else	// MULTI_READ
			{
				SDICCON=(0x1<<9)|(0x1<<8)|0x52;    // sht_resp, wait_resp, dat, start, CMD18
				if(!Chk_CMDend(18, 1))	//-- Check end of CMD18 
					goto RERDCMD;
			}

			SDICSTA=0xa00;	// Clear cmd_end(with rsp)

			while(rd_cnt<128*block);

			INTMSK |= (INT_SDI);
			SDIIMSK=0;	// All mask
			break;

		default:
			break;
	}

    //-- Check end of DATA
	if(!Chk_DATend()) 
		printf("\r\ndat error\r\n");

	SDIDCON = SDIDCON & ~(7<<12);		
	SDIFSTA = SDIFSTA & 0x200;	//Clear Rx FIFO Last data Ready, YH 040221
	SDIDSTA=0x10;	// Clear data Tx/Rx end detect

	if (block > 1)
	{
RERCMD12:    
		//--Stop cmd(CMD12)
		SDICARG=0x0;	    //CMD12(stuff bit)
		SDICCON=(0x1<<9)|(0x1<<8)|0x4c;//sht_resp, wait_resp, start, CMD12

		//-- Check end of CMD12
		if(!Chk_CMDend(12, 1)) 
			goto RERCMD12;
		SDICSTA=0xa00;	// Clear cmd_end(with rsp)
	}
}

int CMD55(void)
{
	//--Make ACMD
	SDICARG=RCA<<16;			//CMD7(RCA,stuff bit)
	SDICCON=(0x1<<9)|(0x1<<8)|0x77;	//sht_resp, wait_resp, start, CMD55

	//-- Check end of CMD55
	if(!Chk_CMDend(55, 1)) 
		return 0;

	SDICSTA=0xa00;	// Clear cmd_end(with rsp)
		return 1;
}

int Chk_SD_OCR(void)
{
	int i;

	//-- Negotiate operating condition for SD, it makes card ready state
	for(i=0;i<50;i++)	//If this time is short, init. can be fail.
	{
		CMD55();    // Make ACMD
		SDICARG=0xff8000;	//ACMD41(SD OCR:2.7V~3.6V)
		//SDICARG=0xffc000;	//ACMD41(MMC OCR:2.6V~3.6V)
		SDICCON=(0x1<<9)|(0x1<<8)|0x69;//sht_resp, wait_resp, start, ACMD41

		//-- Check end of ACMD41
		if( Chk_CMDend(41, 1) & (SDIRSP0 == 0x80ff8000) ) 
		{
			SDICSTA=0xa00;	// Clear cmd_end(with rsp)
			return 1;	// Success	    
		}
		delay(200); // Wait Card power up status
	}
	
	//printf("\r\nSDIRSP0=0x%x\r\n",rSDIRSP0);
	SDICSTA	= 0xa00;	// Clear cmd_end(with rsp)
	return 0;		// Fail
}

void CMD0(void)
{
	//-- Make card idle state 
	SDICARG = 0x0;						// CMD0(stuff bit)
	SDICCON = (0<<9) | (1<<8) | 0x40;	// No_resp, cmd start, CMD0
			
	//-- Check end of CMD0
	Chk_CMDend(0, 0);
	SDICSTA=0x800;	    // Clear cmd_end(no rsp)
}

void SetBus(void)
{
SET_BUS:
	CMD55();	// Make ACMD
	//-- CMD6 implement
	SDICARG=Wide<<1;	    //Wide 0: 1bit, 1: 4bit
	SDICCON=(0x1<<9)|(0x1<<8)|0x46;	//sht_resp, wait_resp, start, CMD55

	if(!Chk_CMDend(6, 1))   // ACMD6
		goto SET_BUS;
	SDICSTA=0xa00;	    // Clear cmd_end(with rsp)
}

void Set_4bit_bus(void)
{
	Wide=1;
	SetBus();
	//printf("\r\n****4bit bus****\r\n");
}

void Set_1bit_bus(void)
{
	Wide=0;
	if(!MMC)
		SetBus();
	//printf("\r\n****1bit bus****\r\n");
}

void Card_sel_desel(char sel_desel)
{
    //-- Card select or deselect
	if(sel_desel) {
RECMDS7:	
		SDICARG=RCA<<16;	// CMD7(RCA,stuff bit)
		SDICCON= (0x1<<9)|(0x1<<8)|0x47;   // sht_resp, wait_resp, start, CMD7

		//-- Check end of CMD7
		if(!Chk_CMDend(7, 1))
			goto RECMDS7;
		SDICSTA=0xa00;	// Clear cmd_end(with rsp)

		//--State(transfer) check
		if( SDIRSP0 & (0x1e00 != 0x800) )
			goto RECMDS7;
	}else {
RECMDD7:	
		SDICARG=0<<16;		//CMD7(RCA,stuff bit)
		SDICCON=(0x1<<8)|0x47;	//no_resp, start, CMD7

		//-- Check end of CMD7
		if(!Chk_CMDend(7, 0))
			goto RECMDD7;
		SDICSTA=0x800;	// Clear cmd_end(no rsp)
	}
}

int sd_init_ll(void)
{
	//-- SD controller & card initialize 
	int i;
	
	SDICON		= (1<<4) | 1;	// Type B, clk enable
	SDIPRE		= PCLK / (INICLK) - 1;	// 400KHz
	SDIFSTA		= SDIFSTA | (1<<16);	//YH 040223 FIFO reset
	SDIBSIZE	= 0x200;		// 512byte(128word)
	SDIDTIMER	= 0x7fffff;		// Set timeout count

	for(i = 0; i < 0x1000; ++i);  // Wait 74SDCLK for MMC card

	CMD0();
	printf("\r\nIn idle state\r\n");

	//-- Check SD card OCR
	if(Chk_SD_OCR()) { 
		printf("\r\nIn SD ready\r\n");
	} else {
		printf("\r\nInitialize fail\r\nNo Card assertion\r\n");
		return 1;
	}

RECMD2:
	//-- Check attaced cards, it makes card identification state
	SDICARG=0x0;   // CMD2(stuff bit)
	SDICCON=(0x1<<10)|(0x1<<9)|(0x1<<8)|0x42; //lng_resp, wait_resp, start, CMD2

	//-- Check end of CMD2
	if(!Chk_CMDend(2, 1)) 
		goto RECMD2;
	SDICSTA=0xa00;	// Clear cmd_end(with rsp)
	
	printf("\r\nEnd id\r\n");

RECMD3:
	//--Send RCA
	SDICARG=MMC<<16;	    // CMD3(MMC:Set RCA, SD:Ask RCA-->SBZ)
	SDICCON=(0x1<<9)|(0x1<<8)|0x43;	// sht_resp, wait_resp, start, CMD3
	
	//-- Check end of CMD3
	if(!Chk_CMDend(3, 1)) 
		goto RECMD3;
	SDICSTA=0xa00;	// Clear cmd_end(with rsp)

	//--Publish RCA
	if(MMC)
	{
		RCA=1;
		SDIPRE=(PCLK/MMCCLK)-1;	
		printf("\r\nMMC Frequency is %dHz\r\n", (u32)(PCLK/(SDIPRE+1)));
	}
	else 
	{
		RCA=( SDIRSP0 & 0xffff0000 )>>16;
		printf("\r\nRCA=0x%x\r\n",RCA);
		SDIPRE=PCLK/(SDCLK)-1;	// Normal clock=25MHz
		//printf("\r\nSD Frequency is %dHz\r\n",(PCLK/(SDIPRE+1)));
	}	    //--State(stand-by) check

	if( (SDIRSP0 & 0x1e00) != 0x600 )  // CURRENT_STATE check
		goto RECMD3;

	printf("\r\nIn stand-by\r\n");

	Card_sel_desel(1);	// Select

	if(!MMC)
		Set_4bit_bus();
	else
		Set_1bit_bus();

	return 0;
}

void Test_SDI(void)
{
	RCA=0;
	MMC=0;
	block=3072;   //3072Blocks=1.5MByte, ((2Block=1024Byte)*1024Block=1MByte)

	GPEUP  = 0xf83f;     // SDCMD, SDDAT[3:0] => PU En.
	GPECON = 0xaaaaaaaa;	//SDCMD, SDDAT[3:0]

	printf("\r\nSDI Card Write and Read Test\r\n");

	/* SD card initialization */
	sd_init_ll();

	printf("\r\nsd_init_ll() is successful!\r\n");

	TR_Buf_new();

	Wt_Block();

	Rd_Block();
	View_Rx_buf();

	Card_sel_desel(0);	// Card deselect

	if(!CMD9())
		printf("\r\nGet CSD fail!!!\r\n");
		
	SDIDCON	= 0;//tark???
	SDICSTA	= 0xffff;
	
	printf("\r\nTest SDI is finished!\r\n");
}


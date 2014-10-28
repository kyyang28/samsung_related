
#include <common.h>

#define MEMCCMD	0x7e001004
#define P1REFRESH	0x7e001010
#define P1CASLAT	0x7e001014
#define MEM_SYS_CFG	0x7e00f120
#define P1MEMCFG	0x7e00100c
#define P1T_DQSS	0x7e001018
#define P1T_MRD		0x7e00101c
#define P1T_RAS		0x7e001020
#define P1T_RC		0x7e001024
#define P1T_RCD		0x7e001028
#define P1T_RFC		0x7e00102c
#define P1T_RP		0x7e001030
#define P1T_RRD		0x7e001034
#define P1T_WR		0x7e001038
#define P1T_WTR		0x7e00103c
#define P1T_XP		0x7e001040
#define P1T_XSR		0x7e001044
#define P1T_ESR		0x7e001048
#define P1MEMCFG2	0X7e00104c
#define P1_chip_0_cfg	0x7e001200

#define P1MEMSTAT	0x7e001000
#define P1MEMCCMD	0x7e001004
#define P1DIRECTCMD	0x7e001008

	
#define HCLK	133000000

#define nstoclk(ns)	(ns/( 1000000000/HCLK)+1)

int ddr_init( void )
{
	// tell dramc to configure				
	set_val( MEMCCMD, 0x4 );

	// set refresh period	
	set_val( P1REFRESH, nstoclk(7800) );

	// set timing para		
	set_val( P1CASLAT, ( 3 << 1 ) );  
	set_val( P1T_DQSS, 0x1 );	// 0.75 - 1.25
	set_val( P1T_MRD, 0x2 );
	set_val( P1T_RAS, nstoclk(45) );
	set_val( P1T_RC, nstoclk(68) );		

	u32 trcd = nstoclk( 23 );
	set_val( P1T_RCD, trcd | (( trcd - 3 ) << 3 ) );
	u32 trfc = nstoclk( 80 );
	set_val( P1T_RFC, trfc | ( ( trfc-3 ) << 5 ) );   
	u32 trp = nstoclk( 23 );
	set_val( P1T_RP, trp | ( ( trp - 3 ) << 3 ) ); 
	set_val( P1T_RRD, nstoclk(15) );
	set_val( P1T_WR, nstoclk(15) );
	set_val( P1T_WTR, 0x7 );
	set_val( P1T_XP, 0x2 );
	set_val( P1T_XSR, nstoclk(120) );
	set_val( P1T_ESR, nstoclk(120) );
	
	// set mem cfg 
	set_nbit( P1MEMCFG, 0, 3, 0x2 );  /* 10 column address */

	/* set_nbit: 把从第bit位开始的一共len位消零，然后把这几位设为val */
	
	set_nbit( P1MEMCFG, 3, 3, 0x2 );  /* 13 row address */
	set_zero( P1MEMCFG, 6 );		  /* A10/AP */
	set_nbit( P1MEMCFG, 15, 3, 0x2 ); /* Burst 4 */
	
	set_nbit( P1MEMCFG2, 0, 4, 0x5 );
	set_2bit( P1MEMCFG2, 6, 0x1 );		/* 32 bit */
	set_nbit( P1MEMCFG2, 8, 3, 0x3 );	/* Mobile DDR SDRAM */
	set_2bit( P1MEMCFG2, 11, 0x1 );

	set_one( P1_chip_0_cfg, 16 );		/* Bank-Row-Column organization */

	// memory init
	set_val( P1DIRECTCMD, 0xc0000 ); // NOP
	set_val( P1DIRECTCMD, 0x000 );	// precharge
	set_val( P1DIRECTCMD, 0x40000 );// auto refresh
	set_val( P1DIRECTCMD, 0x40000 );// auto refresh
	set_val( P1DIRECTCMD, 0xa0000 ); // EMRS
	set_val( P1DIRECTCMD, 0x80032 ); // MRS

	set_val( MEM_SYS_CFG, 0x0 );
					
	// set dramc to "go" status	
	set_val( P1MEMCCMD, 0x000 );

	// wait ready
	while( !(( read_val( P1MEMSTAT ) & 0x3 ) == 0x1));
}




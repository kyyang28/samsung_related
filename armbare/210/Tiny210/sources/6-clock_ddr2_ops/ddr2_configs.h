#ifndef __DDR2_CONFIGS_H
#define __DDR2_CONFIGS_H

/* DMC0 Base Address */
#define DMC0_BASE_ADDR						(0xF0000000)

/* DMC0 Registers Offset */
#define CONCONTROL_OFT						(0x00)
#define MEMCONTROL_OFT						(0x04)
#define MEMCONFIG0_OFT						(0x08)
#define MEMCONFIG1_OFT						(0x0C)
#define DIRECTCMD_OFT						(0x10)
#define PRECHCONFIG_OFT						(0x14)
#define PHYCONTROL0_OFT						(0x18)
#define PHYCONTROL1_OFT						(0x1C)
#define PWRDNCONFIG_OFT						(0x28)
#define TIMINGAREF_OFT						(0x30)
#define TIMINGROW_OFT						(0x34)
#define TIMINGDATA_OFT						(0x38)
#define TIMINGPOWER_OFT						(0x3C)
#define PHYSTATUS_OFT						(0x40)
#define CHIP0STATUS_OFT						(0x48)
#define CHIP1STATUS_OFT						(0x4C)
#define AREFSTATUS_OFT						(0x50)
#define MRSTATUS_OFT						(0x54)
#define PHYTEST0_OFT						(0x58)
#define PHYTEST1_OFT						(0x5C)
#define QOSCONTROL0_OFT						(0x60)
#define QOSCONFIG0_OFT						(0x64)
#define QOSCONTROL1_OFT						(0x68)
#define QOSCONFIG1_OFT						(0x6C)
#define QOSCONTROL2_OFT						(0x70)
#define QOSCONFIG2_OFT						(0x74)
#define QOSCONTROL3_OFT						(0x78)
#define QOSCONFIG3_OFT						(0x7C)
#define QOSCONTROL4_OFT						(0x80)
#define QOSCONFIG4_OFT						(0x84)
#define QOSCONTROL5_OFT						(0x88)
#define QOSCONFIG5_OFT						(0x8C)
#define QOSCONTROL6_OFT						(0x90)
#define QOSCONFIG6_OFT						(0x94)
#define QOSCONTROL7_OFT						(0x98)
#define QOSCONFIG7_OFT						(0x9C)
#define QOSCONTROL8_OFT						(0xA0)
#define QOSCONFIG8_OFT						(0xA4)
#define QOSCONTROL9_OFT						(0xA8)
#define QOSCONFIG9_OFT						(0xAC)
#define QOSCONTROL10_OFT					(0xB0)
#define QOSCONFIG10_OFT						(0xB4)
#define QOSCONTROL11_OFT					(0xB8)
#define QOSCONFIG11_OFT						(0xBC)
#define QOSCONTROL12_OFT					(0xC0)
#define QOSCONFIG12_OFT						(0xC4)
#define QOSCONTROL13_OFT					(0xC8)
#define QOSCONFIG13_OFT						(0xCC)
#define QOSCONTROL14_OFT					(0xD0)
#define QOSCONFIG14_OFT						(0xD4)
#define QOSCONTROL15_OFT					(0xD8)
#define QOSCONFIG15_OFT						(0xDC)

/* PhyControl0 Register Value */
#define CTRL_INC							(0x10)
#define CTRL_START_POINT					(0x10)	
#define CTRL_DLL_OFF						(0x00)		/* Turn off the PHY DLL */
#define CTRL_DLL_ON							(0x01)		/* Turn on the PHY DLL */
#define CTRL_STOP							(0x00)		/* DLL Stop */
#define CTRL_START							(0x01)		/* DLL Start */
#define PHYCONTROL0_DLLON_STOP				((CTRL_INC<<16) | (CTRL_START_POINT<<8) \
											 | (CTRL_DLL_ON<<1) | (CTRL_STOP))
#define PHYCONTROL0_DLLOFF_STOP				((CTRL_INC<<16) | (CTRL_START_POINT<<8) \
											 | (CTRL_DLL_OFF<<1) | (CTRL_STOP))
#define PHYCONTROL0_DLLON					((CTRL_INC<<16) | (CTRL_START_POINT<<8) | (CTRL_DLL_ON<<1))
#define PHYCONTROL0_START					((PHYCONTROL0_DLLON) | (CTRL_START))

/* PhyControl1 Register Value */
#define CTRL_OFFSETC						(0x00)
#define CTRL_REF							(0x08)
#define CTRL_SHIFTC_90SHIFT					(0x05)		/* 0x5 = T/4 (90' shift) */
#define CTRL_SHIFTC_180SHIFT				(0x06)		/* 0x6 = T/2 (180' shift) */
#define PHYCONTROL1_VAL						((CTRL_OFFSETC<<8) | (CTRL_REF<<4) | (CTRL_SHIFTC_180SHIFT))	

/* ConControl Register Value */
#define TIMEOUT_CNT							(0xFFF)
#define RD_FETCH_1CLK						(0x001)
#define RD_FETCH_2CLK						(0x002)
#define AREF_DIS							(0x000)
#define AREF_EN								(0x001)
#define OUT_OF_ORDER_SCHED_DIS				(0x000)
#define OUT_OF_ORDER_SCHED_EN				(0x001)
#define CONCONTROL_AREF_DIS					((TIMEOUT_CNT<<16) | (RD_FETCH_2CLK<<12) | (AREF_DIS<<5) \
											 | (OUT_OF_ORDER_SCHED_EN<<4))
#define CONCONTROL_AREF_EN					((TIMEOUT_CNT<<16) | (RD_FETCH_2CLK<<12) | (AREF_EN<<5) \
											 | (OUT_OF_ORDER_SCHED_EN<<4))

/* MemControl Register Value */
#define MEM_BURST_LEN_2BL					(0x01)
#define MEM_BURST_LEN_4BL					(0x02)
#define MEM_BURST_LEN_8BL					(0x03)
#define MEM_BURST_LEN_16BL					(0x04)
#define MEM_WIDTH_16BIT						(0x01)
#define MEM_WIDTH_32BIT						(0x02)
#define MEM_TYPE_DDR2						(0x04)
#define DPWRDN_TYPE_ACTIVE					(0x00)
#define DPWRDN_TYPE_FORCE					(0x01)
#define DPWRDN_DIS							(0x00)
#define DPWRDN_EN							(0x01)
#define CLK_ALWAYS_RUNNING					(0x00)
#define CLK_STOPS_DURING_IDLE				(0x01)
#define MEMCONTROL_VAL						((MEM_BURST_LEN_4BL<<20) | (MEM_WIDTH_32BIT<<12) \
											 | (MEM_TYPE_DDR2<<8) | (DPWRDN_TYPE_ACTIVE<<2) \
											 | (DPWRDN_DIS<<1) | (CLK_ALWAYS_RUNNING))

/* MemConfig0 Register Value */
#define CHIP_BASE							(0x20)
#define CHIP_BASE_CONFIG1					(0x00)
#define CHIP_MASK							(0xE0)	/* 0xE0 => AXI Base addr = 0x0 ~ 0x1FFFFFFF */
#define CHIP_MAP_LINEAR						(0x00)
#define CHIP_MAP_INTERLEAVED				(0x01)
#define CHIP_COL_10BITS						(0x03)
#define CHIP_COL_11BITS						(0x04)
#define CHIP_ROW_13BITS						(0x01)
#define CHIP_ROW_14BITS						(0x02)
#define CHIP_BANK_4BANKS					(0x02)
#define CHIP_BANK_8BANKS					(0x03)
#define MEMCONFIG0_VAL						((CHIP_BASE<<24) | (CHIP_MASK<<16) | (CHIP_MAP_LINEAR<<12) \
											 | (CHIP_COL_10BITS<<8) | (CHIP_ROW_14BITS<<4) \
											 | (CHIP_BANK_8BANKS))
#define MEMCONFIG1_VAL						((CHIP_BASE_CONFIG1<<24) | (CHIP_MASK<<16) \
											 | (CHIP_MAP_LINEAR<<12) | (CHIP_COL_10BITS<<8) \
											 | (CHIP_ROW_14BITS<<4) | (CHIP_BANK_8BANKS))

/* PrechConfig Register Value */
#define TP_CNT								(0xFF)
#define CHIP1_OPEN_PAGE_POLICY				(0x00)	/* Open page policy */
#define CHIP1_CLOSE_PAGE_POLICY				(0x01)	/* Close page (auto precharge) policy */
#define CHIP0_OPEN_PAGE_POLICY				(0x00)	/* Open page policy */
#define CHIP0_CLOSE_PAGE_POLICY				(0x01)	/* Close page (auto precharge) policy */
#define PRECHCONFIG_VAL						((TP_CNT<<24) | (CHIP1_OPEN_PAGE_POLICY<<8) \
											 | (CHIP0_OPEN_PAGE_POLICY))	

/* PwrdnConfig Register Value */
#define DSREF_CYC							(0xFFFF)
#define DPWRDN_CYC							(0x00FF)
#define PWRDNCONFIG_VAL						((DSREF_CYC<<16) | (DPWRDN_CYC))

/* TimingAref Register Value */
#define T_REFI								(1560)	/* 7.8us * 200MHz = 1560 = 0x618 */
#define TIMINGAREF_VAL						(T_REFI)

/* TimingRow Register Value */
#define T_RFC								(21) /* tRFC = 105ns, 1clk = 5ns => T_RFC = 105/5 = 21clk */
#define T_RRD								(2)	 /* tRRD = 7.5ns, 1clk = 5ns => T_RRD = 7.5/5 = 1.5clk */
#define T_RP								(3)	 /* tRP = 15ns, 1clk = 5ns => T_RP = 15/5 = 3clk */
#define T_RCD								(3)	 /* tRCD = 15ns, 1clk = 5ns => T_RCD = 15/5 = 3clk */
#define T_RC								(12) /* tRC = 60ns, 1clk = 5ns => T_RC = 60/5 = 12clk */
#define T_RAS								(9)	 /* tRAS = 45ns, 1clk = 5ns => T_RAS = 45/5 = 9clk */
#define TIMINGROW_VAL						((T_RFC<<24) | (T_RRD<<20) | (T_RP<<16) \
											 | (T_RCD<<12) | (T_RC<<6) | (T_RAS))

/* TimingData Register Value */
#define T_WTR								(2)	 /* tWTR = 7.5ns, 1clk = 5ns => T_WTR = 7.5/5 = 1.5clk */
#define T_WR								(3)	 /* tWR = 15ns, 1clk = 5ns => T_WR = 15/5 = 3clk */
#define	T_RTP								(2)	 /* tRTP = 7.5ns, 1clk = 5ns => T_RTP = 7.5/5 = 1.5clk */ 
#define CL									(4)	 /* CL_old = 6 */
#define TIMINGDATA_VAL						((T_WTR<<28) | (T_WR<<24) | (T_RTP<<20) | (CL<<16))

/* TimingPower Register Value */
#define T_FAW								(10) /* tFAW = 50ns, 1clk = 5ns => T_FAW = 50/5 = 10clk */
#define T_XSR								(200)
#define T_XP								(2)
#define T_CKE								(3)
#define T_MRD								(2)
#define TIMINGPOWER_VAL						((T_FAW<<24) | (T_XSR<<16) | (T_XP<<8) | (T_CKE<<4) | (T_MRD))

/* DirectCmd Register Value */
#define CMD_TYPE_MRS_CHIP0					((0x0<<24) | (0x0<<16))
#define CMD_TYPE_MRS_CHIP1					((0x0<<24) | (0x1<<20) | (0x0<<16))	/* 0x1<<20 => Chip 1 */
#define BURST_LEN_4							(0x2)
#define BURST_LEN_8							(0x3)
#define BURST_TYPE_SEQ						(0x0)
#define BURST_TYPE_INTER					(0x1)
#define CAS_2								(0x2)
#define CAS_3								(0x3)
#define CAS_4								(0x4)
#define CAS_5								(0x5)
#define CAS_6								(0x6)
#define CAS_7								(0x7)
#define MODE_NORMAL							(0x0)
#define MODE_TEST							(0x1)
#define DLL_NO_RESET						(0x0)
#define DLL_RESET							(0x1)
#define WR_APRE_2							(0x1)	/* WR = Write Recovery */
#define WR_APRE_3							(0x2)
#define APD_FASTEXIT						(0x0)	/* APD = Active Power Down */
#define APD_SLOWEXIT						(0x1)
#define MRS_SETTINGS_DLLRESET_CHIP0			((CMD_TYPE_MRS_CHIP0<<16) | (APD_FASTEXIT<<12) \
											 | (WR_APRE_3<<9) | (DLL_RESET<<8) | (MODE_NORMAL<<7) \
											 | (CAS_4<<4) | (BURST_TYPE_SEQ<<3) | (BURST_LEN_4))
#define MRS_SETTINGS_DLLNORESET_CHIP0		((CMD_TYPE_MRS_CHIP0<<16) | (APD_FASTEXIT<<12) \
											 | (WR_APRE_3<<9) | (DLL_NO_RESET<<8) \
											 | (MODE_NORMAL<<7) | (CAS_4<<4) \
											 | (BURST_TYPE_SEQ<<3) | (BURST_LEN_4))

#define MRS_SETTINGS_DLLRESET_CHIP1			((CMD_TYPE_MRS_CHIP1<<16) | (APD_FASTEXIT<<12) \
											 | (WR_APRE_3<<9) | (DLL_RESET<<8) | (MODE_NORMAL<<7) \
											 | (CAS_4<<4) | (BURST_TYPE_SEQ<<3) | (BURST_LEN_4))
#define MRS_SETTINGS_DLLNORESET_CHIP1		((CMD_TYPE_MRS_CHIP1<<16) | (APD_FASTEXIT<<12) \
											 | (WR_APRE_3<<9) | (DLL_NO_RESET<<8) | (MODE_NORMAL<<7) \
											 | (CAS_4<<4) | (BURST_TYPE_SEQ<<3) | (BURST_LEN_4))

#define CMD_TYPE_EMRS_CHIP0					((0x0<<24) | (0x1<<16))
#define CMD_TYPE_EMRS_CHIP1					((0x0<<24) | (0x1<<20) | (0x1<<16))
#define CMD_TYPE_EMRS2_CHIP0				((0x0<<24) | (0x2<<16))
#define CMD_TYPE_EMRS2_CHIP1				((0x0<<24) | (0x1<<20) | (0x2<<16))
#define CMD_TYPE_EMRS3_CHIP0				((0x0<<24) | (0x3<<16))
#define CMD_TYPE_EMRS3_CHIP1				((0x0<<24) | (0x1<<20) | (0x3<<16))
#define EMRS_DLL_EN							(0x0)
#define EMRS_DLL_DIS						(0x1)
#define EMRS_OCD_CALI_MODE_EXIT				(0x0)
#define EMRS_OCD_CALI_DEFAULT				(0x7)
#define EMRS_DQS_EN							(0x0)
#define EMRS_DQS_DIS						(0x1)
#define EMRS_SETTINGS_DLLEN_DQSDIS_CHIP0	((CMD_TYPE_EMRS_CHIP0) | (EMRS_DQS_DIS<<10) | (EMRS_DLL_EN<<0))
#define EMRS_SETTINGS_OCD_DEFAULT_CHIP0		((CMD_TYPE_EMRS_CHIP0) | (EMRS_OCD_CALI_DEFAULT<<7) \
											 | (EMRS_DQS_DIS<<10) | (EMRS_DLL_EN<<0))
#define EMRS_SETTINGS_OCD_EXIT_CHIP0		((CMD_TYPE_EMRS_CHIP0) | (EMRS_OCD_CALI_MODE_EXIT<<7) \
											 | (EMRS_DQS_DIS<<10) | (EMRS_DLL_EN<<0))

#define EMRS_SETTINGS_DLLEN_DQSDIS_CHIP1	((CMD_TYPE_EMRS_CHIP1) | (EMRS_DQS_DIS<<10) | (EMRS_DLL_EN<<0))
#define EMRS_SETTINGS_OCD_DEFAULT_CHIP1		((CMD_TYPE_EMRS_CHIP1) | (EMRS_OCD_CALI_DEFAULT<<7) \
											 | (EMRS_DQS_DIS<<10) | (EMRS_DLL_EN<<0))
#define EMRS_SETTINGS_OCD_EXIT_CHIP1		((CMD_TYPE_EMRS_CHIP1) | (EMRS_OCD_CALI_MODE_EXIT<<7) \
											 | (EMRS_DQS_DIS<<10) | (EMRS_DLL_EN<<0))

#define CMD_TYPE_PALL_CHIP0					(0x1<<24)
#define CMD_TYPE_PRE_CHIP0					(0x2<<24)
#define CMD_TYPE_DPD_CHIP0					(0x3<<24)
#define CMD_TYPE_REFS_CHIP0					(0x4<<24)
#define CMD_TYPE_REFA_CHIP0					(0x5<<24)
#define CMD_TYPE_CKEL_CHIP0					(0x6<<24)
#define CMD_TYPE_NOP_CHIP0					(0x7<<24)
#define CMD_TYPE_REFSX_CHIP0				(0x8<<24)
#define CMD_TYPE_MRR_CHIP0					(0x9<<24)

#define CMD_TYPE_PALL_CHIP1					((0x1<<24) | (0x1<<20))
#define CMD_TYPE_PRE_CHIP1					((0x2<<24) | (0x1<<20))
#define CMD_TYPE_DPD_CHIP1					((0x3<<24) | (0x1<<20))
#define CMD_TYPE_REFS_CHIP1					((0x4<<24) | (0x1<<20))
#define CMD_TYPE_REFA_CHIP1					((0x5<<24) | (0x1<<20))
#define CMD_TYPE_CKEL_CHIP1					((0x6<<24) | (0x1<<20))
#define CMD_TYPE_NOP_CHIP1					((0x7<<24) | (0x1<<20))
#define CMD_TYPE_REFSX_CHIP1				((0x8<<24) | (0x1<<20))
#define CMD_TYPE_MRR_CHIP1					((0x9<<24) | (0x1<<20))

#endif


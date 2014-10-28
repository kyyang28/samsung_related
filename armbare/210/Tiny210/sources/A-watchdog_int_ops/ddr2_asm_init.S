
#include "ddr2_configs.h"

.globl ddr2_asm_init
ddr2_asm_init:
	/* Load the DMC0_BASE_ADDR to r0 */
	ldr	r0, =DMC0_BASE_ADDR

	/* SEQ 2: Setup the PhyControl0 Register to stop the DLL */
	ldr	r1, =PHYCONTROL0_DLLON_STOP
	str r1, [r0, #PHYCONTROL0_OFT]

	/* SEQ 3: Setup the PhyControl1 Register */
	ldr r1, =PHYCONTROL1_VAL
	str r1, [r0, #PHYCONTROL1_OFT]

	/* SEQ 4: Setup the PhyControl0 Register to start the DLL */
	ldr r1, =PHYCONTROL0_START
	str r1, [r0, #PHYCONTROL0_OFT]

	/* SEQ 5: Setup the ConControl Register to disable auto refresh counter */
	ldr r1, =CONCONTROL_AREF_DIS
	str r1, [r0, #CONCONTROL_OFT]

	/* SEQ 6: Setup the MemControl Register */
	ldr r1, =MEMCONTROL_VAL
	str r1, [r0, #MEMCONTROL_OFT]

	/* SEQ 7: Setup the MemConfig0 Register */
	ldr r1, =MEMCONFIG0_VAL
	str r1, [r0, #MEMCONFIG0_OFT]

	/* SEQ 7: Setup the MemConfig1 Register */
	ldr r1, =MEMCONFIG1_VAL
	str r1, [r0, #MEMCONFIG1_OFT]

	/* SEQ 8: Setup the PrechConfig Register */
	ldr	r1, =PRECHCONFIG_VAL
	str r1, [r0, #PRECHCONFIG_OFT]

	/* SEQ 8: Setup the PwrdnConfig Register */
	ldr r1, =PWRDNCONFIG_VAL
	str r1, [r0, #PWRDNCONFIG_OFT]

	/* SEQ 9: Setup the TimingAref Register */
	ldr	r1, =TIMINGAREF_VAL
	str r1, [r0, #TIMINGAREF_OFT]

	/* SEQ 9: Setup the TimingRow Register */
	ldr	r1, =TIMINGROW_VAL
	str r1, [r0, #TIMINGROW_OFT]

	/* SEQ 9: Setup the TimingData Register */
	ldr	r1, =TIMINGDATA_VAL
	str r1, [r0, #TIMINGDATA_OFT]

	/* SEQ 9: Setup the TimingPower Register */
	ldr	r1, =TIMINGPOWER_VAL
	str r1, [r0, #TIMINGPOWER_OFT]

	/* 
	 * SEQ 11: Wait for the PhyStatus0.ctrl_locked bit-fields to change to ‘1’. 
	 *		   Check whether PHY DLL is locked 
	 */
check_dll_locked:
	/* Loop until DLL is locked */
	ldr r1, [r0, #PHYSTATUS_OFT]
	and r2, r1, #0x7
	cmp r2, #0x7
	bne check_dll_locked
	
	/* Chip 0 Device Memory DDR2 DRAM Initialization */
	/* SEQ 14: Issue a NOP command using the DirectCmd Register */
	ldr	r1, =CMD_TYPE_NOP_CHIP0
	str r1, [r0, #DIRECTCMD_OFT]

	/* SEQ 16: Issue a PALL command using the DirectCmd Register */
	ldr r1, =CMD_TYPE_PALL_CHIP0
	str r1, [r0, #DIRECTCMD_OFT]

	/* SEQ 17: Issue an EMRS2 command using the DirectCmd Register */
	ldr r1, =CMD_TYPE_EMRS2_CHIP0
	str r1, [r0, #DIRECTCMD_OFT]

	/* SEQ 18: Issue an EMRS3 command using the DirectCmd Register */
	ldr r1, =CMD_TYPE_EMRS3_CHIP0
	str r1, [r0, #DIRECTCMD_OFT]

	/* SEQ 19: Issue an EMRS command using the DirectCmd Register to enable DLL and disable DQS */
	ldr r1, =EMRS_SETTINGS_DLLEN_DQSDIS_CHIP0
	str r1, [r0, #DIRECTCMD_OFT]

	/* SEQ 20: Issue an MRS command using the DirectCmd Register to reset the memory DLL */
	ldr r1, =MRS_SETTINGS_DLLRESET_CHIP0
	str r1, [r0, #DIRECTCMD_OFT]

	/* SEQ 21: Issue a PALL command using the DirectCmd Register */
	ldr r1, =CMD_TYPE_PALL_CHIP0
	str r1, [r0, #DIRECTCMD_OFT]

	/* SEQ 22: Issue two Auto Refresh commands using the DirectCmd Register */
	ldr r1, =CMD_TYPE_REFA_CHIP0
	str r1, [r0, #DIRECTCMD_OFT]
	ldr r1, =CMD_TYPE_REFA_CHIP0
	str r1, [r0, #DIRECTCMD_OFT]

	/* 
	 * SEQ 23: Issue a MRS command using the DirectCmd register to program 
	 * the operating parameters without resetting the memory DLL
	 */
	ldr r1, =MRS_SETTINGS_DLLNORESET_CHIP0
	str r1, [r0, #DIRECTCMD_OFT]

	/* 
	 * SEQ 25: Issue an EMRS command using the DirectCmd register to program 
	 * the operating parameters. If OCD calibration is not used,
	 * issue an EMRS command to set OCD Calibration Default. After that,
	 * issue an EMRS command to exit OCD Calibration Mode and to program
	 * the operating parameters.
	 */
	ldr	r1, =EMRS_SETTINGS_OCD_DEFAULT_CHIP0
	str r1, [r0, #DIRECTCMD_OFT]

	ldr r1, =EMRS_SETTINGS_OCD_EXIT_CHIP0
	str r1, [r0, #DIRECTCMD_OFT]

	/* Chip 1 Device Memory DDR2 DRAM Initialization */
	/* SEQ 26: Issue a NOP command using the DirectCmd Register */
	ldr	r1, =CMD_TYPE_NOP_CHIP1
	str r1, [r0, #DIRECTCMD_OFT]

	/* Issue a PALL command using the DirectCmd Register */
	ldr r1, =CMD_TYPE_PALL_CHIP1
	str r1, [r0, #DIRECTCMD_OFT]

	/* Issue an EMRS2 command using the DirectCmd Register */
	ldr r1, =CMD_TYPE_EMRS2_CHIP1
	str r1, [r0, #DIRECTCMD_OFT]

	/* Issue an EMRS3 command using the DirectCmd Register */
	ldr r1, =CMD_TYPE_EMRS3_CHIP1
	str r1, [r0, #DIRECTCMD_OFT]

	/* Issue an EMRS command using the DirectCmd Register to enable DLL and disable DQS */
	ldr r1, =EMRS_SETTINGS_DLLEN_DQSDIS_CHIP1
	str r1, [r0, #DIRECTCMD_OFT]

	/* Issue an MRS command using the DirectCmd Register to reset the memory DLL */
	ldr r1, =MRS_SETTINGS_DLLRESET_CHIP1
	str r1, [r0, #DIRECTCMD_OFT]

	/* Issue a PALL command using the DirectCmd Register */
	ldr r1, =CMD_TYPE_PALL_CHIP1
	str r1, [r0, #DIRECTCMD_OFT]

	/* Issue two Auto Refresh commands using the DirectCmd Register */
	ldr r1, =CMD_TYPE_REFA_CHIP1
	str r1, [r0, #DIRECTCMD_OFT]
	ldr r1, =CMD_TYPE_REFA_CHIP1
	str r1, [r0, #DIRECTCMD_OFT]

	/* 
	 * Issue a MRS command using the DirectCmd register to program 
	 * the operating parameters without resetting the memory DLL
	 */
	ldr r1, =MRS_SETTINGS_DLLNORESET_CHIP1
	str r1, [r0, #DIRECTCMD_OFT]

	/* 
	 * Issue an EMRS command using the DirectCmd register to program 
	 * the operating parameters. If OCD calibration is not used,
	 * issue an EMRS command to set OCD Calibration Default. After that,
	 * issue an EMRS command to exit OCD Calibration Mode and to program
	 * the operating parameters.
	 */
	ldr	r1, =EMRS_SETTINGS_OCD_DEFAULT_CHIP1
	str r1, [r0, #DIRECTCMD_OFT]

	ldr r1, =EMRS_SETTINGS_OCD_EXIT_CHIP1
	str r1, [r0, #DIRECTCMD_OFT]

	/* Setup the ConControl to turn on an auto refresh counter */
	ldr r1, =CONCONTROL_AREF_EN
	str r1, [r0, #CONCONTROL_OFT]

	/* SEQ 27: If power down modes is required, set the MemControl register */

	mov pc, lr


/*
 * $Id: config.h,v 1.7 2005/02/09 09:23:55 pavlov Exp $
 *
 * Faked config options for out-of-kernel-tree build
 */

#ifndef __MTD_CONFIG_H__

#include_next <linux/config.h>

#ifdef MTD_OUT_OF_TREE

#define CONFIG_NFTL_RW 1

#define CONFIG_MTD_MAP_BANK_WIDTH_1 1
#define CONFIG_MTD_MAP_BANK_WIDTH_2 1
#define CONFIG_MTD_MAP_BANK_WIDTH_4 1
#define CONFIG_MTD_CFI_I1 1
#define CONFIG_MTD_CFI_I2 1
#define CONFIG_MTD_CFI_I4 1

#ifndef SIMPLEMAP
#define CONFIG_MTD_COMPLEX_MAPPINGS 1
#endif

#define CONFIG_MTD_PARTITIONS 1

#define CONFIG_MTD_PHYSMAP_START 0x8000000
#define CONFIG_MTD_PHYSMAP_LEN 0x4000000
#define CONFIG_MTD_PHYSMAP_BUSWIDTH 2

#define CONFIG_MTDRAM_TOTAL_SIZE 4096
#define CONFIG_MTDRAM_ERASE_SIZE 64

#endif /* MTD_OUT_OF_TREE */

#ifndef NONAND
#define CONFIG_JFFS2_FS_WRITEBUFFER 1
#endif

#endif /* __MTD_CONFIG_H__ */

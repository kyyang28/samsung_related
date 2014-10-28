/*
 * JFFS3 -- Journalling Flash File System, Version 3.
 *
 * Copyright (C) 2001-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@infradead.org>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * $Id: debug.h,v 1.23 2005/01/25 20:11:12 hammache Exp $
 */
#ifndef __JFFS3_DEBUG_H__
#define __JFFS3_DEBUG_H__

#include <linux/config.h>
#include <linux/kernel.h>
#include "jffs3.h"

#ifndef CONFIG_JFFS3_FS_DEBUG
#define CONFIG_JFFS3_FS_DEBUG 1
#endif

#if CONFIG_JFFS3_FS_DEBUG > 0
#define DEBUG1		1
#define PARANOIA	1
#else
#define DEBUG1		0
#define PARANOIA	0
#endif

#if CONFIG_JFFS3_FS_DEBUG > 1
#define DEBUG2	1
#else
#define DEBUG2	0
#endif

/*
 * Sanity checks are supposed to be quick and are always enabled. When the correspondent
 * part of JFFS3 is conciedered stable, the sanity check is moved to paranoid check and is
 * disabled by default. If it is wanted to make JFFS3 a bit faster, sanity checks may be disabled.
 */
#define SANITY	1

/*
 * TODO: While actively developing JFFS3, keep PARANOIA defined in order
 * to catch bugs on early stage. This section must be removed later.
 */
#if SANITY > 0
#undef PARANOIA
#define PARANOIA 1
#endif

/*
 * JFFS3 subsystems. Used in messaging to distinguish debugging messages
 * from different parts of JFFS3 and to be able to enable/disable
 * messages from different parts of JFFS3 when developing/debugging
 * JFFS3.
 * Another objective is to have standard output format with prefixed
 * function name, etc. When developing, one may add something else
 * (PID, etc).
 *
 * If developing something one may define new subsystem with its own
 * output as well as to split some of already existing, if it is
 * cosiedered to be too large.
 */
#define JFFS3_DBG_OTHER		1	/* Not classified */
#define JFFS3_DBG_SCAN		2	/* Flash scan */
#define JFFS3_DBG_GC		3	/* Garbage Collector */
#define JFFS3_DBG_WBUF		4	/* Write buffer */
#define JFFS3_DBG_ERASE		5	/* Block erase */
#define JFFS3_DBG_SALLOC	6	/* Main data structures allocation */
#define JFFS3_DBG_READ		7	/* JFFS2 nodes reading */
#define JFFS3_DBG_GCT		8	/* GC thread */
#define JFFS3_DBG_VFS		9	/* VFS callback handlers */
#define JFFS3_DBG_BLD		10	/* File system build (on mount) */
#define JFFS3_DBG_COMPR		11	/* Compression */
#define JFFS3_DBG_SUMMARY	12	/* Summaries processing */
#define JFFS3_DBG_RI		13	/* Read inode */
#define JFFS3_DBG_BI		14	/* Build inode's fragtree/dirents list  */
#define JFFS3_DBG_RSV		15	/* Flash space reservation */
#define JFFS3_DBG_NR		16	/* Manipulating raw_node_ref objects */
#define JFFS3_DBG_BL		17	/* Manipulating block lists */
#define JFFS3_DBG_WRITE		18	/* JFFS2 nodes creation and writing */

#if CONFIG_JFFS3_FS_DEBUG > 0
/* By default print the debugging messages from almost all JFFS3 subsystems */
#define JFFS3_DBG_SUBSYS_OTHER_PRINT	1
#define JFFS3_DBG_SUBSYS_SCAN_PRINT	1
#define JFFS3_DBG_SUBSYS_GC_PRINT	1
#define JFFS3_DBG_SUBSYS_WBUF_PRINT	1
#define JFFS3_DBG_SUBSYS_ERASE_PRINT	1
#define JFFS3_DBG_SUBSYS_SALLOC_PRINT	0
#define JFFS3_DBG_SUBSYS_READ_PRINT	1
#define JFFS3_DBG_SUBSYS_GCT_PRINT	1
#define JFFS3_DBG_SUBSYS_VFS_PRINT	1
#define JFFS3_DBG_SUBSYS_BLD_PRINT	1
#define JFFS3_DBG_SUBSYS_COMPR_PRINT	1
#define JFFS3_DBG_SUBSYS_SUMMARY_PRINT	1
#define JFFS3_DBG_SUBSYS_RI_PRINT	1
#define JFFS3_DBG_SUBSYS_BI_PRINT	1
#define JFFS3_DBG_SUBSYS_RSV_PRINT	1
#define JFFS3_DBG_SUBSYS_NR_PRINT	1
#define JFFS3_DBG_SUBSYS_BL_PRINT	1
#define JFFS3_DBG_SUBSYS_WRITE_PRINT	1
#else
#define JFFS3_DBG_SUBSYS_OTHER_PRINT	0
#define JFFS3_DBG_SUBSYS_SCAN_PRINT	0
#define JFFS3_DBG_SUBSYS_GC_PRINT	0
#define JFFS3_DBG_SUBSYS_WBUF_PRINT	0
#define JFFS3_DBG_SUBSYS_ERASE_PRINT	0
#define JFFS3_DBG_SUBSYS_SALLOC_PRINT	0
#define JFFS3_DBG_SUBSYS_READ_PRINT	0
#define JFFS3_DBG_SUBSYS_GCT_PRINT	0
#define JFFS3_DBG_SUBSYS_VFS_PRINT	0
#define JFFS3_DBG_SUBSYS_BLD_PRINT	0
#define JFFS3_DBG_SUBSYS_COMPR_PRINT	0
#define JFFS3_DBG_SUBSYS_SUMMARY_PRINT	0
#define JFFS3_DBG_SUBSYS_RI_PRINT	0
#define JFFS3_DBG_SUBSYS_BI_PRINT	0
#define JFFS3_DBG_SUBSYS_RSV_PRINT	0
#define JFFS3_DBG_SUBSYS_NR_PRINT	0
#define JFFS3_DBG_SUBSYS_BL_PRINT	0
#define JFFS3_DBG_SUBSYS_WRITE_PRINT	0
#endif

/*
 * When developing JFFS3 it is often useful to enable debugging messages
 * for some particular JFFS3 subsystem. This function prints message
 * only if the messages for the specified JFFS3 subsystem are enabled.
 */
#define JFFS3DBG_SUBSYSTEM(subsystem, debug_level, args...)			\
do {										\
	char *subsysname = NULL;						\
	if ((debug_level) <= CONFIG_JFFS3_FS_DEBUG) {				\
		switch (subsystem) {						\
			case JFFS3_DBG_WRITE:					\
				if (JFFS3_DBG_SUBSYS_WRITE_PRINT)		\
					subsysname = "write ";			\
				break;						\
			case JFFS3_DBG_NR:					\
				if (JFFS3_DBG_SUBSYS_NR_PRINT)			\
					subsysname = "nref  ";			\
				break;						\
			case JFFS3_DBG_BL:					\
				if (JFFS3_DBG_SUBSYS_BL_PRINT)			\
					subsysname = "bldlst";			\
				break;						\
			case JFFS3_DBG_RSV:					\
				if (JFFS3_DBG_SUBSYS_RSV_PRINT)			\
					subsysname = "reserv";			\
				break;						\
			case JFFS3_DBG_BI:					\
				if (JFFS3_DBG_SUBSYS_BI_PRINT)			\
					subsysname = "bldino";			\
				break;						\
			case JFFS3_DBG_RI:					\
				if (JFFS3_DBG_SUBSYS_RI_PRINT)			\
					subsysname = "rdino ";			\
				break;						\
			case JFFS3_DBG_SUMMARY:					\
				if (JFFS3_DBG_SUBSYS_SUMMARY_PRINT)		\
					subsysname = "summ  ";			\
				break;						\
			case JFFS3_DBG_COMPR:					\
				if (JFFS3_DBG_SUBSYS_COMPR_PRINT)		\
					subsysname = "compr ";			\
				break;						\
			case JFFS3_DBG_BLD:					\
				if (JFFS3_DBG_SUBSYS_BLD_PRINT)			\
					subsysname = "build ";			\
				break;						\
			case JFFS3_DBG_VFS:					\
				if (JFFS3_DBG_SUBSYS_VFS_PRINT)			\
					subsysname = "VFS   ";			\
				break;						\
			case JFFS3_DBG_GCT:					\
				if (JFFS3_DBG_SUBSYS_GCT_PRINT)			\
					subsysname = "GCT   ";			\
				break;						\
			case JFFS3_DBG_READ:					\
				if (JFFS3_DBG_SUBSYS_READ_PRINT)		\
					subsysname = "read  ";			\
				break;						\
			case JFFS3_DBG_SALLOC:					\
				if (JFFS3_DBG_SUBSYS_SALLOC_PRINT)		\
					subsysname = "salloc";			\
				break;						\
			case JFFS3_DBG_ERASE:					\
				if (JFFS3_DBG_SUBSYS_ERASE_PRINT)		\
					subsysname = "erase ";			\
				break;						\
			case JFFS3_DBG_SCAN:					\
				if (JFFS3_DBG_SUBSYS_SCAN_PRINT)		\
					subsysname = "scan  ";			\
				break;						\
			case JFFS3_DBG_GC:					\
				if (JFFS3_DBG_SUBSYS_GC_PRINT)			\
					subsysname = "GC    ";			\
				break;						\
			case JFFS3_DBG_WBUF:					\
				if (JFFS3_DBG_SUBSYS_GC_PRINT)			\
					subsysname = "wbuf  ";			\
				break;						\
			case JFFS3_DBG_OTHER:					\
				if (JFFS3_DBG_SUBSYS_OTHER_PRINT)		\
					subsysname = "other ";			\
				break;						\
			default:						\
				BUG();						\
		}								\
		if (subsysname) {						\
			printk(KERN_DEBUG "[JFFS3 DBG%d ", debug_level);	\
			printk("%s] ", subsysname);				\
			printk("%s(): ", __FUNCTION__);				\
			printk(args);						\
		}								\
	}									\
} while(0)

/*
 * All outputs with the following macros are supposed to be removed by the
 * compiler if the JFFS3 debugging is disabled.
 */
#define DBG(debug_level, args...)						\
	JFFS3DBG_SUBSYSTEM(JFFS3_DBG_OTHER, debug_level, args)

#define DBG_SCAN(debug_level, args...)						\
	JFFS3DBG_SUBSYSTEM(JFFS3_DBG_SCAN, debug_level, args)

#define DBG_GC(debug_level, args...)						\
	JFFS3DBG_SUBSYSTEM(JFFS3_DBG_GC, debug_level, args)

#define DBG_WBUF(debug_level, args...)						\
	JFFS3DBG_SUBSYSTEM(JFFS3_DBG_WBUF, debug_level, args)

#define DBG_ERASE(debug_level, args...)						\
	JFFS3DBG_SUBSYSTEM(JFFS3_DBG_ERASE, debug_level, args)

#define DBG_SALLOC(debug_level, args...)					\
	JFFS3DBG_SUBSYSTEM(JFFS3_DBG_SALLOC, debug_level, args)

#define DBG_READ(debug_level, args...)						\
	JFFS3DBG_SUBSYSTEM(JFFS3_DBG_READ, debug_level, args)

#define DBG_GCT(debug_level, args...)						\
	JFFS3DBG_SUBSYSTEM(JFFS3_DBG_GCT, debug_level, args)

#define DBG_VFS(debug_level, args...)						\
	JFFS3DBG_SUBSYSTEM(JFFS3_DBG_VFS, debug_level, args)

#define DBG_BLD(debug_level, args...)						\
	JFFS3DBG_SUBSYSTEM(JFFS3_DBG_BLD, debug_level, args)

#define DBG_COMPR(debug_level, args...)						\
	JFFS3DBG_SUBSYSTEM(JFFS3_DBG_COMPR, debug_level, args)

#define DBG_SUMMARY(debug_level, args...)					\
	JFFS3DBG_SUBSYSTEM(JFFS3_DBG_SUMMARY, debug_level, args)

#define DBG_RI(debug_level, args...)						\
	JFFS3DBG_SUBSYSTEM(JFFS3_DBG_RI, debug_level, args)

#define DBG_BI(debug_level, args...)						\
	JFFS3DBG_SUBSYSTEM(JFFS3_DBG_BI, debug_level, args)

#define DBG_RSV(debug_level, args...)						\
	JFFS3DBG_SUBSYSTEM(JFFS3_DBG_RSV, debug_level, args)

#define DBG_NR(debug_level, args...)						\
	JFFS3DBG_SUBSYSTEM(JFFS3_DBG_NR, debug_level, args)

#define DBG_BL(debug_level, args...)						\
	JFFS3DBG_SUBSYSTEM(JFFS3_DBG_BL, debug_level, args)

#define DBG_WRITE(debug_level, args...)						\
	JFFS3DBG_SUBSYSTEM(JFFS3_DBG_WRITE, debug_level, args)

/*
 * Please, use this macro in case of critical error which makes JFFS3
 * futher unworkable or when you use BUG().
 */
#define ERROR_MSG(args...)							\
	do {									\
		printk(KERN_ERR "[JFFS3] ERROR! %s(): ", __FUNCTION__);		\
		printk(args);							\
	} while(0)

/*
 * Please, use this macro in case of errors which do not make JFFS3
 * unworkable.
 */
#define WARNING_MSG(args...)							\
	do {									\
		printk(KERN_WARNING "[JFFS3] WARNING! %s(): ", __FUNCTION__);	\
		printk(args);							\
	} while(0)

/*
 * Use this macro when you want to note something. Do not use too often
 * in order not to put unneded garbage to the system log.
 */
#define NOTICE_MSG(args...)							\
	do {									\
		printk(KERN_NOTICE "[JFFS3] ");					\
		printk(args);							\
	} while(0)

void
jffs3_dbg_acct_paranoia_check(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb);

void
jffs3_dbg_prewrite_paranoia_check(struct jffs3_sb_info *c, uint32_t ofs, int len);

void
jffs3_dbg_fragtree_paranoia_check(struct jffs3_inode_info *f);

void
jffs3_dbg_dump_block_lists(struct jffs3_sb_info *c);

void
jffs3_dbg_dump_frag_list(struct jffs3_inode_info *f);

void
jffs3_dbg_dump_dirents_list(struct jffs3_full_dirent *list);

void
jffs3_dbg_dump_buffer(char *buf, int len, uint32_t offs);

void
jffs3_dbg_dump_node_refs(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb);


#if SANITY > 0
/*
 * Check the space accounting of the file system and of the JFFS3 erasable block 'jeb'.
 */
static inline void
jffs3_dbg_acct_sanity_check(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb)
{
	if (unlikely(jeb && jeb->used_size + jeb->dirty_size + jeb->free_size
				+ jeb->wasted_size + jeb->unchecked_size != c->sector_size)) {
		ERROR_MSG("Eeep. Space accounting for block at 0x%08x is screwed\n", jeb->offset);
		printk(KERN_ERR "free %#08x + dirty %#08x + used %#08x + wasted %#08x + unchecked "
				"%#08x != total %#08x\n", jeb->free_size, jeb->dirty_size, jeb->used_size,
				jeb->wasted_size, jeb->unchecked_size, c->sector_size);
		BUG();
	}

	if (unlikely(c->used_size + c->dirty_size + c->free_size + c->erasing_size + c->bad_size
				+ c->wasted_size + c->unchecked_size != c->flash_size)) {
		ERROR_MSG("Eeep. Space accounting superblock info is screwed\n");
		printk(KERN_ERR "free %#08x + dirty %#08x + used %#08x + erasing %#08x + bad %#08x + "
				"wasted %#08x + unchecked %#08x != total %#08x\n",
				c->free_size, c->dirty_size, c->used_size, c->erasing_size, c->bad_size,
				c->wasted_size, c->unchecked_size, c->flash_size);
		BUG();
	}
}

/*
 * Check the wbuf accounting.
 */
static inline void
jffs3_dbg_wbuf_acct_sanity_check(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb)
{
	if (jeb->free_size < (c->wbuf_pagesize - c->wbuf_len)) {
		ERROR_MSG("Accounting error. wbuf at %#08x has %#03x bytes, %#03x left.\n",
		       c->wbuf_ofs, c->wbuf_len, c->wbuf_pagesize-c->wbuf_len);
		ERROR_MSG("But free_size for block at %#08x is only %#08x\n",
		       jeb->offset, jeb->free_size);
		BUG();
	}
}

/*
 * When adding new raw_node_ref, check that it is added just after
 * previous node.
 */
static inline int
jffs3_dbg_node_add_sanity_check(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb,
				struct jffs3_raw_node_ref *ref, uint32_t ref_len)
{
	/*
	 * During mounting c->nextblock may be uninitialized, so don't
	 * check it if JFFS3_SB_FLAG_MOUNTING flag is set.
	 */
	/* we could get some obsolete nodes after nextblock was refiled
	   in wbuf.c */
	if ((c->nextblock || !ref_obsolete(ref))
	    && (jeb != c->nextblock
		|| ref_offset(ref) != jeb->offset + (c->sector_size - jeb->free_size)) ) {
		WARNING_MSG("Argh. Node added in wrong place.\n");
		printk(KERN_ERR "c->nextblock %p, jeb %p, new offs %#08x, new len %#x, "
			"should be %#08x (jeb->offset %#08x, c->sector_size %#x, "
			"jeb->free_size %#x)\n", c->nextblock, jeb,
			ref_offset(ref), ref_len, jeb->offset + (c->sector_size - jeb->free_size),
			jeb->offset, c->sector_size, jeb->free_size);
		return -1;
	}

	return 0;
}
#endif

#endif /* __JFFS3_DEBUG_H__ */

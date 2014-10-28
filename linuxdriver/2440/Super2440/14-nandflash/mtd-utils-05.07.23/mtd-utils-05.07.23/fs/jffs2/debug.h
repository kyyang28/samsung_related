/*
 * JFFS2 -- Journalling Flash File System, Version 2.
 *
 * Copyright (C) 2001-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@infradead.org>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * $Id: debug.h,v 1.4 2005/07/22 10:32:08 dedekind Exp $
 *
 */
#ifndef _JFFS2_DEBUG_H_
#define _JFFS2_DEBUG_H_

#include <linux/config.h>

#ifndef CONFIG_JFFS2_FS_DEBUG
#define CONFIG_JFFS2_FS_DEBUG 1
#endif

/* Enable "paranoia" checks and dumps if debugging is on */
#if CONFIG_JFFS2_FS_DEBUG > 0
#define JFFS2_DBG_PARANOIA_CHECKS
#define JFFS2_DBG_DUMPS
#endif

/* Enable JFFS2 sanity checks by default */
#define JFFS2_DBG_SANITY_CHECKS

/* 
 * Dx() are mainly used for debugging messages, they must go away and be
 * superseded by nicer macros.
 */
#if CONFIG_JFFS2_FS_DEBUG > 0
#define D1(x) x
#else
#define D1(x)
#endif

#if CONFIG_JFFS2_FS_DEBUG > 1
#define D2(x) x
#else
#define D2(x)
#endif

/* "Paranoia" checks */
void
__jffs2_dbg_fragtree_paranoia_check(struct jffs2_inode_info *f);
void
__jffs2_dbg_fragtree_paranoia_check_nolock(struct jffs2_inode_info *f);
void
__jffs2_dbg_acct_paranoia_check(struct jffs2_sb_info *c,
			   	struct jffs2_eraseblock *jeb);
void
__jffs2_dbg_acct_paranoia_check_nolock(struct jffs2_sb_info *c,
				       struct jffs2_eraseblock *jeb);
void
__jffs2_dbg_prewrite_paranoia_check(struct jffs2_sb_info *c,
				    uint32_t ofs, int len);

/* "Dump" functions */
void
__jffs2_dbg_dump_jeb(struct jffs2_sb_info *c, struct jffs2_eraseblock *jeb);
void
__jffs2_dbg_dump_jeb_nolock(struct jffs2_eraseblock *jeb);
void
__jffs2_dbg_dump_block_lists(struct jffs2_sb_info *c);
void
__jffs2_dbg_dump_block_lists_nolock(struct jffs2_sb_info *c);
void
__jffs2_dbg_dump_node_refs(struct jffs2_sb_info *c,
		 	   struct jffs2_eraseblock *jeb);
void
__jffs2_dbg_dump_node_refs_nolock(struct jffs2_sb_info *c,
				  struct jffs2_eraseblock *jeb);
void
__jffs2_dbg_dump_fragtree(struct jffs2_inode_info *f);
void
__jffs2_dbg_dump_fragtree_nolock(struct jffs2_inode_info *f);
void
__jffs2_dbg_dump_buffer(unsigned char *buf, int len, uint32_t offs);

#ifdef JFFS2_DBG_PARANOIA_CHECKS
#define jffs2_dbg_fragtree_paranoia_check(f)			\
	__jffs2_dbg_fragtree_paranoia_check(f)
#define jffs2_dbg_fragtree_paranoia_check_nolock(f)		\
	__jffs2_dbg_fragtree_paranoia_check_nolock(f)
#define jffs2_dbg_acct_paranoia_check(c, jeb)			\
	__jffs2_dbg_acct_paranoia_check(c,jeb)
#define jffs2_dbg_acct_paranoia_check_nolock(c, jeb)		\
	__jffs2_dbg_acct_paranoia_check_nolock(c,jeb)
#define jffs2_dbg_prewrite_paranoia_check(c, ofs, len)		\
	__jffs2_dbg_prewrite_paranoia_check(c, ofs, len)
#else
#define jffs2_dbg_fragtree_paranoia_check(f)
#define jffs2_dbg_fragtree_paranoia_check_nolock(f)
#define jffs2_dbg_acct_paranoia_check(c, jeb)
#define jffs2_dbg_acct_paranoia_check_nolock(c, jeb)
#define jffs2_dbg_prewrite_paranoia_check(c, ofs, len)
#endif /* !JFFS2_PARANOIA_CHECKS */

#ifdef JFFS2_DBG_DUMPS
#define jffs2_dbg_dump_jeb(c, jeb)				\
	__jffs2_dbg_dump_jeb(c, jeb);
#define jffs2_dbg_dump_jeb_nolock(jeb)				\
	__jffs2_dbg_dump_jeb_nolock(jeb);
#define jffs2_dbg_dump_block_lists(c)				\
	__jffs2_dbg_dump_block_lists(c)
#define jffs2_dbg_dump_block_lists_nolock(c)			\
	__jffs2_dbg_dump_block_lists_nolock(c)
#define jffs2_dbg_dump_fragtree(f)				\
	__jffs2_dbg_dump_fragtree(f);
#define jffs2_dbg_dump_fragtree_nolock(f)			\
	__jffs2_dbg_dump_fragtree_nolock(f);
#define jffs2_dbg_dump_buffer(buf, len, offs)			\
	__jffs2_dbg_dump_buffer(*buf, len, offs);
#else
#define jffs2_dbg_dump_jeb(c, jeb)
#define jffs2_dbg_dump_jeb_nolock(jeb)
#define jffs2_dbg_dump_block_lists(c)
#define jffs2_dbg_dump_block_lists_nolock(c)
#define jffs2_dbg_dump_fragtree(f)
#define jffs2_dbg_dump_fragtree_nolock(f)
#define jffs2_dbg_dump_buffer(buf, len, offs)
#endif /* !JFFS2_DBG_DUMPS */

#ifdef JFFS2_DBG_SANITY_CHECKS
/*
 * Check the space accounting of the file system and of
 * the JFFS2 erasable block 'jeb'.
 */
static inline void
jffs2_dbg_acct_sanity_check_nolock(struct jffs2_sb_info *c,
				   struct jffs2_eraseblock *jeb)
{
	if (unlikely(jeb && jeb->used_size + jeb->dirty_size +
			jeb->free_size + jeb->wasted_size +
			jeb->unchecked_size != c->sector_size)) {
		printk(KERN_ERR "Eeep. Space accounting for block at 0x%08x is screwed\n", jeb->offset);
		printk(KERN_ERR "free %#08x + dirty %#08x + used %#08x + wasted %#08x + unchecked "
				"%#08x != total %#08x\n", jeb->free_size, jeb->dirty_size, jeb->used_size,
				jeb->wasted_size, jeb->unchecked_size, c->sector_size);
		BUG();
	}

	if (unlikely(c->used_size + c->dirty_size + c->free_size + c->erasing_size + c->bad_size
				+ c->wasted_size + c->unchecked_size != c->flash_size)) {
		printk(KERN_ERR "Eeep. Space accounting superblock info is screwed\n");
		printk(KERN_ERR "free %#08x + dirty %#08x + used %#08x + erasing %#08x + bad %#08x + "
				"wasted %#08x + unchecked %#08x != total %#08x\n",
				c->free_size, c->dirty_size, c->used_size, c->erasing_size, c->bad_size,
				c->wasted_size, c->unchecked_size, c->flash_size);
		BUG();
	}
}

static inline void
jffs2_dbg_acct_sanity_check(struct jffs2_sb_info *c,
			    struct jffs2_eraseblock *jeb)
{
	spin_lock(&c->erase_completion_lock);
	jffs2_dbg_acct_sanity_check_nolock(c, jeb);
	spin_unlock(&c->erase_completion_lock);
}
#else
#define jffs2_dbg_acct_sanity_check(c, jeb)
#define jffs2_dbg_acct_sanity_check_nolock(c, jeb)
#endif /* !JFFS2_DBG_SANITY_CHECKS */

#endif /* _JFFS2_DEBUG_H_ */

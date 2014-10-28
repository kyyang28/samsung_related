/*
 * JFFS3 -- Journalling Flash File System, Version 3.
 *
 * Copyright (C) 2001-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@infradead.org>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * $Id: scan.h,v 1.7 2005/02/09 14:52:05 pavlov Exp $
 */
#ifndef __JFFS3_SCAN_H__
#define __JFFS3_SCAN_H__

#include <linux/config.h>
#include <linux/kernel.h>
#include "jffs3.h"
#include "nodelist.h"

/* Old style. Should go. */
#define DIRTY_SPACE(x) do { typeof(x) _x = (x); \
		c->free_size -= _x; c->dirty_size += _x; \
		jeb->free_size -= _x ; jeb->dirty_size += _x; \
		}while(0)
#define USED_SPACE(x) do { typeof(x) _x = (x); \
		c->free_size -= _x; c->used_size += _x; \
		jeb->free_size -= _x ; jeb->used_size += _x; \
		}while(0)
#define WASTED_SPACE(x) do { typeof(x) _x = (x); \
		c->free_size -= _x; c->wasted_size += _x; \
		jeb->free_size -= _x ; jeb->wasted_size += _x; \
		}while(0)
#define UNCHECKED_SPACE(x) do { typeof(x) _x = (x); \
		c->free_size -= _x; c->unchecked_size += _x; \
		jeb->free_size -= _x ; jeb->unchecked_size += _x; \
		}while(0)

#define BLK_STATE_ALLFF		0
#define BLK_STATE_CLEAN		1
#define BLK_STATE_PARTDIRTY	2
#define BLK_STATE_CLEANMARKER	3
#define BLK_STATE_ALLDIRTY	4
#define BLK_STATE_BADBLOCK	5

#define DEFAULT_EMPTY_SCAN_SIZE 1024

#define DBG_NOISY(noise, args...) do { \
	if (*(noise)) { \
		NOTICE_MSG(args); \
		(*(noise))--; \
		if (!(*(noise))) \
			NOTICE_MSG("Further such events for this erase block will not be printed\n"); \
	} \
} while(0)

#define JFFS3_SUMMARY_NOSUM_SIZE 0xffffffff
#define JFFS3_SUMMARY_INODE_SIZE (sizeof(struct jffs3_sum_inode_flash))
#define JFFS3_SUMMARY_DIRENT_SIZE(x) (sizeof(struct jffs3_sum_dirent_flash) + (x))

/*
 * Mark space dirty. Each chunk of dirty space in JFFS3 should be
 * represented by the correspondent obsolete node (in order to be
 * able to calculate nodes length in the ref_totlen() function).
 * 
 * If the last node is obsolete, do not allocate ne node_ref object,
 * just merge it with our dirt.
 */
static inline int
dirty_space(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb, uint32_t size)
{
		struct jffs3_raw_node_ref *ref;

		size = PAD(size);

		DBG_SCAN(2, "Add dirt of length %#x to the node_refs list\n", size);

		/* 
		 * If the previous node represents dirt too, don't
		 * allocate new node_ref object.
		 */
		if (jeb->last_node && ref_obsolete(jeb->last_node)) {
			DBG_SCAN(2, "Previous node at %#08x is obsolete too, "
					"don't allocate new node_ref\n",
					ref_offset(jeb->last_node));
#ifdef TMP_TOTLEN
			jeb->last_node->__totlen += size;
#endif
			c->free_size -= size;
			jeb->free_size -= size;
			jeb->dirty_size += size;
			c->dirty_size += size;
			return 0;
		}

		ref = jffs3_alloc_raw_node_ref();
		if (!ref) {
			WARNING_MSG("Can't allocate raw_node_ref object\n");
			return -ENOMEM;
		}

		ref->flash_offset = (jeb->offset + (c->sector_size - jeb->free_size)) | REF_OBSOLETE;
		ref->next_in_ino = ref->next_phys = NULL;
#ifdef TMP_TOTLEN
		ref->__totlen = size;
#endif

		if (!jeb->first_node)
			jeb->first_node = ref;
		if (jeb->last_node)
			jeb->last_node->next_phys = ref;
		jeb->last_node = ref;
		
		c->free_size -= size;
		jeb->free_size -= size;
		c->dirty_size += size;
		jeb->dirty_size += size;
		
		return 0;
}

static inline uint32_t EMPTY_SCAN_SIZE(uint32_t sector_size) {
	if (sector_size < DEFAULT_EMPTY_SCAN_SIZE)
		return sector_size;
	else
		return DEFAULT_EMPTY_SCAN_SIZE;
}

#endif /* __JFFS3_SCAN_H__ */

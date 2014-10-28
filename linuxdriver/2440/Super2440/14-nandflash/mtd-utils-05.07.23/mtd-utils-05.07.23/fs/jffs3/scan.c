/*
 * JFFS3 -- Journalling Flash File System, Version 3.
 *
 * Copyright (C) 2001-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@infradead.org>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * JFFS3 Id: scan.c,v 1.115 2004/11/17 12:59:08 dedekind  Exp
 * $Id: scan.c,v 3.18 2005/05/09 08:16:09 havasi Exp $
 *
 */
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/mtd/mtd.h>
#include <linux/pagemap.h>
#include <linux/crc32.h>
#include <linux/compiler.h>
#include "nodelist.h"
#include "summary.h"
#include "scan.h"

static uint32_t pseudo_random;

static int jffs3_scan_eraseblock (struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb,
				  unsigned char *buf, uint32_t buf_size);

/* These helper functions _must_ increase ofs and also do the dirty/used space accounting.
 * Returning an error will abort the mount - bad checksums etc. should just mark the space
 * as dirty.
 */
static int jffs3_scan_inode_node(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb,
				 struct jffs3_raw_inode *ri, uint32_t ofs);
static int jffs3_scan_dirent_node(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb,
				 struct jffs3_raw_dirent *rd, uint32_t ofs);

static inline int min_free(struct jffs3_sb_info *c)
{
	uint32_t min = 2 * sizeof(struct jffs3_raw_inode);
#ifdef CONFIG_JFFS3_FS_WRITEBUFFER
	if (!jffs3_can_mark_obsolete(c) && min < c->wbuf_pagesize)
		return c->wbuf_pagesize;
#endif
	return min;

}
int jffs3_scan_medium(struct jffs3_sb_info *c)
{
	int i, ret;
	uint32_t empty_blocks = 0, bad_blocks = 0;
	unsigned char *flashbuf = NULL;
	uint32_t buf_size = 0;
#ifndef __ECOS
	size_t pointlen;

	if (c->mtd->point) {
		ret = c->mtd->point (c->mtd, 0, c->mtd->size, &pointlen, &flashbuf);
		if (!ret && pointlen < c->mtd->size) {
			/* Don't muck about if it won't let us point to the whole flash */
			DBG_SCAN(1, "MTD point returned len too short: %#zx\n", pointlen);
			c->mtd->unpoint(c->mtd, flashbuf, 0, c->mtd->size);
			flashbuf = NULL;
		}
		if (ret)
			DBG_SCAN(1, "MTD point failed with return code %d\n", ret);
	}
#endif
	if (!flashbuf) {
		/* For NAND it's quicker to read a whole eraseblock at a time,
		   apparently */
		if (jffs3_cleanmarker_oob(c))
			buf_size = c->sector_size;
		else
			buf_size = PAGE_SIZE;

		/* Respect kmalloc limitations */
		if (buf_size > 128*1024)
			buf_size = 128*1024;

		DBG_SCAN(1, "Allocating readbuf of %d bytes\n", buf_size);
		flashbuf = kmalloc(buf_size, GFP_KERNEL);
		if (!flashbuf)
			return -ENOMEM;
	}

	for (i=0; i<c->nr_blocks; i++) {
		struct jffs3_eraseblock *jeb = &c->blocks[i];

		ret = jffs3_scan_eraseblock(c, jeb, buf_size?flashbuf:(flashbuf+jeb->offset), buf_size);

		if (ret < 0)
			goto out;

		if (PARANOIA)
			jffs3_dbg_acct_paranoia_check(c, jeb);

		/* Now decide which list to put it on */
		switch(ret) {
		case BLK_STATE_ALLFF:
			/*
			 * Empty block.   Since we can't be sure it
			 * was entirely erased, we just queue it for erase
			 * again.  It will be marked as such when the erase
			 * is complete.  Meanwhile we still count it as empty
			 * for later checks.
			 */
			empty_blocks++;
			list_add(&jeb->list, &c->erase_pending_list);
			c->nr_erasing_blocks++;
			break;

		case BLK_STATE_CLEANMARKER:
			/* Only a CLEANMARKER node is valid */
			if (!jeb->dirty_size) {
				/* It's actually free */
				list_add(&jeb->list, &c->free_list);
				c->nr_free_blocks++;
			} else {
				/* Dirt */
				DBG_SCAN(1, "Adding all-dirty block at %#08x to "
						"erase_pending_list\n", jeb->offset);
				list_add(&jeb->list, &c->erase_pending_list);
				c->nr_erasing_blocks++;
			}
			break;

		case BLK_STATE_CLEAN:
                        /* Full (or almost full) of clean data. Clean list */
                        list_add(&jeb->list, &c->clean_list);
			break;

		case BLK_STATE_PARTDIRTY:
                        /* Some data, but not full. Dirty list. */
                        /* We want to remember the block with most free space
                           and stick it in the 'nextblock' position to start writing to it. */
                        if (jeb->free_size > min_free(c) &&
			    (!c->nextblock || c->nextblock->free_size < jeb->free_size)) {
                                /* Better candidate for the next writes to go to */
                                if (c->nextblock) {
					c->nextblock->dirty_size += c->nextblock->free_size + c->nextblock->wasted_size;
					c->dirty_size += c->nextblock->free_size + c->nextblock->wasted_size;
					c->free_size -= c->nextblock->free_size;
					c->wasted_size -= c->nextblock->wasted_size;
					c->nextblock->free_size = c->nextblock->wasted_size = 0;
					if (VERYDIRTY(c, c->nextblock->dirty_size)) {
						list_add(&c->nextblock->list, &c->very_dirty_list);
					} else {
						list_add(&c->nextblock->list, &c->dirty_list);
					}
				}
                                c->nextblock = jeb;
                        } else {
				jeb->dirty_size += jeb->free_size + jeb->wasted_size;
				c->dirty_size += jeb->free_size + jeb->wasted_size;
				c->free_size -= jeb->free_size;
				c->wasted_size -= jeb->wasted_size;
				jeb->free_size = jeb->wasted_size = 0;
				if (VERYDIRTY(c, jeb->dirty_size)) {
					list_add(&jeb->list, &c->very_dirty_list);
				} else {
					list_add(&jeb->list, &c->dirty_list);
				}
                        }
			break;

		case BLK_STATE_ALLDIRTY:
			/* Nothing valid - not even a clean marker. Needs erasing. */
                        /* For now we just put it on the erasing list. We'll start the erases later */
			DBG_SCAN(1, "Erase block at %#08x is not formatted. It will "
					"be erased\n", jeb->offset);
			list_add(&jeb->list, &c->erase_pending_list);
			c->nr_erasing_blocks++;
			break;

		case BLK_STATE_BADBLOCK:
			DBG_SCAN(1, "Block at %#08x is bad\n", jeb->offset);
			list_add(&jeb->list, &c->bad_list);
			c->bad_size += c->sector_size;
			c->free_size -= c->sector_size;
			bad_blocks++;
			break;
		default:
			ERROR_MSG("Unknown block state\n");
			BUG();
		}
	}

	/* Nextblock dirty is always seen as wasted, because we cannot recycle it now */
	if (c->nextblock && (c->nextblock->dirty_size)) {
		c->nextblock->wasted_size += c->nextblock->dirty_size;
		c->wasted_size += c->nextblock->dirty_size;
		c->dirty_size -= c->nextblock->dirty_size;
		c->nextblock->dirty_size = 0;
	}
#ifdef CONFIG_JFFS3_FS_WRITEBUFFER
	if (!jffs3_can_mark_obsolete(c) && c->nextblock && (c->nextblock->free_size & (c->wbuf_pagesize-1))) {
		/* If we're going to start writing into a block which already
		   contains data, and the end of the data isn't page-aligned,
		   skip a little and align it. */

		uint32_t skip = c->nextblock->free_size & (c->wbuf_pagesize-1);

		DBG_SCAN(1, "Skipping %d bytes in nextblock to ensure page alignment\n", skip);
		c->nextblock->wasted_size += skip;
		c->wasted_size += skip;

		c->nextblock->free_size -= skip;
		c->free_size -= skip;
	}
#endif
	if (c->nr_erasing_blocks) {
		if (!c->used_size && ((c->nr_free_blocks + empty_blocks+bad_blocks)
					!= c->nr_blocks || bad_blocks == c->nr_blocks) ) {
			NOTICE_MSG("Cowardly refusing to erase blocks on filesystem with "
					"no valid JFFS3 nodes\n");
			NOTICE_MSG("Empty blocks %d, bad blocks %d, total blocks %d\n",
					empty_blocks,bad_blocks,c->nr_blocks);
			ret = -EIO;
			goto out;
		}
		jffs3_erase_pending_trigger(c);
	}
	ret = 0;
 out:
	if (buf_size)
		kfree(flashbuf);
#ifndef __ECOS
	else
		c->mtd->unpoint(c->mtd, flashbuf, 0, c->mtd->size);
#endif
	return ret;
}

int jffs3_fill_scan_buf (struct jffs3_sb_info *c, unsigned char *buf,
				uint32_t ofs, uint32_t len)
{
	int ret;
	size_t retlen;

	ret = jffs3_flash_read(c, ofs, len, &retlen, buf);
	if (ret) {
		WARNING_MSG("mtd->read(%#08x bytes from %#08x) returned %d\n", len, ofs, ret);
		return ret;
	}
	if (retlen < len) {
		WARNING_MSG("Read at %#08x gave only 0x%zx bytes\n", ofs, retlen);
		return -EIO;
	}

	DBG_SCAN(2, "Read %#08x bytes from %#08x into buf\n", len, ofs);
	DBG_SCAN(2, "000: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x "
				"%02x %02x %02x %02x %02x %02x\n", buf[0], buf[1],
				buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9],
				buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
	return 0;
}

static int jffs3_scan_eraseblock (struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb,
				  unsigned char *buf, uint32_t buf_size) {
	struct jffs3_unknown_node *node;
	struct jffs3_unknown_node crcnode;
	uint32_t ofs, prevofs;
	uint32_t hdr_crc, buf_ofs, buf_len;
	int err;
	int noise = 0;

#ifdef CONFIG_JFFS3_SUMMARY
	struct jffs3_sum_marker *sm;
#endif

#ifdef CONFIG_JFFS3_FS_WRITEBUFFER
	int cleanmarkerfound = 0;
#endif

	ofs = jeb->offset;
	prevofs = jeb->offset - 1;

	DBG_SCAN(1, "Scanning block at %#x\n", ofs);

#ifdef CONFIG_JFFS3_FS_WRITEBUFFER
	if (jffs3_cleanmarker_oob(c)) {
		int ret = jffs3_check_nand_cleanmarker(c, jeb);
		DBG_SCAN(1, "jffs_check_nand_cleanmarker returned %d\n", ret);
		/* Even if it's not found, we still scan to see
		   if the block is empty. We use this information
		   to decide whether to erase it or not. */
		switch (ret) {
		case 0:		cleanmarkerfound = 1; break;
		case 1: 	break;
		case 2: 	return BLK_STATE_BADBLOCK;
		case 3:		return BLK_STATE_ALLDIRTY; /* Block has failed to erase min. once */
		default: 	return ret;
		}
	}
#endif

#ifdef CONFIG_JFFS3_SUMMARY
	sm = (struct jffs3_sum_marker *)kmalloc(sizeof(struct jffs3_sum_marker), GFP_KERNEL);
	if (!sm) {
	    return -ENOMEM;
	}

	err = jffs3_fill_scan_buf(c, (unsigned char *) sm, jeb->offset + c->sector_size - sizeof(struct jffs3_sum_marker), sizeof(struct jffs3_sum_marker));

	if (err) {
		kfree(sm);
	        return err;
	}

	if (je32_to_cpu(sm->magic) == JFFS3_SUM_MAGIC ) {

		if(je32_to_cpu(sm->erase_size) == c->sector_size) {
			int ret = jffs3_sum_scan_sumnode(c,jeb,je32_to_cpu(sm->offset),&pseudo_random);

			if (ret) {
				kfree(sm);
				return ret;
			}
		}

		WARNING_MSG("FS erase_block_size != JFFS3 erase_block_size => skipping summary information\n");

	}

	kfree(sm);

	ofs = jeb->offset;
	prevofs = jeb->offset - 1;

#endif

	buf_ofs = jeb->offset;

	if (!buf_size) {
		buf_len = c->sector_size;

#ifdef CONFIG_JFFS3_SUMMARY
		/* must reread because of summary test */
		err = jffs3_fill_scan_buf(c, buf, buf_ofs, buf_len);
		if (err)
			return err;
#endif

	} else {
		buf_len = EMPTY_SCAN_SIZE(c->sector_size);
		err = jffs3_fill_scan_buf(c, buf, buf_ofs, buf_len);
		if (err)
			return err;
	}

	/* We temporarily use 'ofs' as a pointer into the buffer/jeb */
	ofs = 0;

	/* Scan only 4KiB of 0xFF before declaring it's empty */
	while(ofs < EMPTY_SCAN_SIZE(c->sector_size) && *(uint32_t *)(&buf[ofs]) == 0xFFFFFFFF)
		ofs += 4;

	if (ofs == EMPTY_SCAN_SIZE(c->sector_size)) {
#ifdef CONFIG_JFFS3_FS_WRITEBUFFER
		if (jffs3_cleanmarker_oob(c)) {
			/* scan oob, take care of cleanmarker */
			int ret = jffs3_check_oob_empty(c, jeb, cleanmarkerfound);
			DBG_SCAN(2, "jffs3_check_oob_empty returned %d\n", ret);
			switch (ret) {
			case 0:		return cleanmarkerfound ? BLK_STATE_CLEANMARKER : BLK_STATE_ALLFF;
			case 1: 	return BLK_STATE_ALLDIRTY;
			default: 	return ret;
			}
		}
#endif
		DBG_SCAN(1, "Block at %#08x is empty (erased\n", jeb->offset);
		return BLK_STATE_ALLFF;	/* OK to erase if all blocks are like this */
	}
	if (ofs) {
		DBG_SCAN(2, "Free space at %#08x ends at %#08x\n",
				jeb->offset, jeb->offset + ofs);
		if (dirty_space(c, jeb, ofs) != 0)
			return -ENOMEM;
	}

	/* Now ofs is a complete physical flash offset as it always was... */
	ofs += jeb->offset;

	noise = 10;

#ifdef CONFIG_JFFS3_SUMMARY
	DBG_SUMMARY(1,"no summary found in jeb 0x%08x. Apply original scan.\n", jeb->offset);
#endif

scan_more:
	while(ofs < jeb->offset + c->sector_size) {

		if (PARANOIA)
			jffs3_dbg_acct_paranoia_check(c, jeb);

		cond_resched();

		if (SANITY && ofs & 3) {
			WARNING_MSG("Eep. ofs %#08x not word-aligned!\n", ofs);
			ofs = PAD(ofs);
			continue;
		}

		if (SANITY && ofs == prevofs) {
			WARNING_MSG("ofs %#08x has already been seen. Skipping\n", ofs);
			if (dirty_space(c, jeb, 4) != 0)
				return -ENOMEM;
			ofs += 4;
			continue;
		}

		prevofs = ofs;

		if (jeb->offset + c->sector_size < ofs + sizeof(*node)) {
			DBG_SCAN(1, "Fewer than %zd bytes left to end of block. "
					"(%#08x+%#08x<%#08x+%zx Not reading\n",
					sizeof(struct jffs3_unknown_node),
					jeb->offset, c->sector_size, ofs, sizeof(*node));
			if (dirty_space(c, jeb, (jeb->offset + c->sector_size) - ofs) != 0)
				return -ENOMEM;
			break;
		}

		if (buf_ofs + buf_len < ofs + sizeof(*node)) {
			buf_len = min_t(uint32_t, buf_size, jeb->offset + c->sector_size - ofs);
			DBG_SCAN(1, "Fewer than %zd bytes (node header left to end "
					"of buf. Reading %#08x at %#08x\n",
					sizeof(struct jffs3_unknown_node), buf_len, ofs);
			err = jffs3_fill_scan_buf(c, buf, ofs, buf_len);
			if (err)
				return err;
			buf_ofs = ofs;
		}

		node = (struct jffs3_unknown_node *)&buf[ofs-buf_ofs];

		if (*(uint32_t *)(&buf[ofs-buf_ofs]) == 0xffffffff) {
			uint32_t inbuf_ofs;
			uint32_t empty_start;

			empty_start = ofs;
			ofs += 4;

			DBG_SCAN(1, "Found empty flash at %#08x\n", ofs);
		more_empty:
			inbuf_ofs = ofs - buf_ofs;
			while (inbuf_ofs < buf_len) {
				if (*(uint32_t *)(&buf[inbuf_ofs]) != 0xffffffff) {
					WARNING_MSG("Empty flash at %#08x ends at %#08x\n",
						       empty_start, ofs);
					if (dirty_space(c, jeb, ofs - empty_start) != 0)
						return -ENOMEM;
					goto scan_more;
				}

				inbuf_ofs+=4;
				ofs += 4;
			}
			/* Ran off end. */
			DBG_SCAN(1, "Empty flash to end of buffer at %#08x\n", ofs);

			/* If we're only checking the beginning of a block with a cleanmarker,
			   bail now */
			if (buf_ofs == jeb->offset && jeb->used_size == PAD(c->cleanmarker_size) &&
			    c->cleanmarker_size && !jeb->dirty_size && !jeb->first_node->next_phys) {
				DBG_SCAN(1, "%d bytes at start of block seems clean... "
						"assuming all clean\n", EMPTY_SCAN_SIZE(c->sector_size));
				return BLK_STATE_CLEANMARKER;
			}

			/* See how much more there is to read in this eraseblock... */
			buf_len = min_t(uint32_t, buf_size, jeb->offset + c->sector_size - ofs);
			if (!buf_len) {
				/* No more to read. Break out of main loop without marking
				   this range of empty space as dirty (because it's not) */
				DBG_SCAN(1, "Empty flash at %#08x runs to end of "
						"block. Treating as free_space\n", empty_start);
				break;
			}
			DBG_SCAN(1, "Reading another %#08x at %#08x\n", buf_len, ofs);
			err = jffs3_fill_scan_buf(c, buf, ofs, buf_len);
			if (err)
				return err;
			buf_ofs = ofs;
			goto more_empty;
		}

		if (ofs == jeb->offset && je16_to_cpu(node->magic) == KSAMTIB_CIGAM_2SFFJ) {
			WARNING_MSG("Magic bitmask is backwards at offset %#08x. "
					"Wrong endian filesystem?\n", ofs);
			if (dirty_space(c, jeb, 4) != 0)
				return -ENOMEM;
			ofs += 4;
			continue;
		}

		if (je16_to_cpu(node->magic) == JFFS3_DIRTY_BITMASK) {
			DBG_SCAN(1, "Dirty bitmask at %#08x\n", ofs);
			if (dirty_space(c, jeb, 4) != 0)
				return -ENOMEM;
			ofs += 4;
			continue;
		}

		if (je16_to_cpu(node->magic) != JFFS3_MAGIC_BITMASK) {
			/* OK. We're out of possibilities. Whinge and move on */
			DBG_NOISY(&noise, "Magic bitmask 0x%04x not found at %#08x: 0x%04x instead\n",
				     JFFS3_MAGIC_BITMASK, ofs,
				     je16_to_cpu(node->magic));
			if (dirty_space(c, jeb, 4) != 0)
				return -ENOMEM;
			ofs += 4;
			continue;
		}
		/* We seem to have a node of sorts. Check the CRC */
		crcnode.magic = node->magic;
		crcnode.nodetype = cpu_to_je16( je16_to_cpu(node->nodetype) | JFFS3_NODE_ACCURATE);
		crcnode.totlen = node->totlen;
		hdr_crc = crc32(0, &crcnode, sizeof(crcnode)-4);

		if (hdr_crc != je32_to_cpu(node->hdr_crc)) {
			DBG_NOISY(&noise, "Node at %#08x {0x%04x, 0x%04x, %#08x) has "
					"invalid CRC %#08x (calculated %#08x)\n",
					ofs, je16_to_cpu(node->magic), je16_to_cpu(node->nodetype),
					je32_to_cpu(node->totlen), je32_to_cpu(node->hdr_crc), hdr_crc);
			if (dirty_space(c, jeb, 4) != 0)
				return -ENOMEM;
			ofs += 4;
			continue;
		}

		if (ofs + je32_to_cpu(node->totlen) >
		    jeb->offset + c->sector_size) {
			/* Eep. Node goes over the end of the erase block. */
			WARNING_MSG("Node at %#08x with length %#08x would run over the "
					"end of the erase block\n", ofs, je32_to_cpu(node->totlen));
			WARNING_MSG("Perhaps the file system was created ""with the wrong erase size?\n");
			if (dirty_space(c, jeb, 4) != 0)
				return -ENOMEM;
			ofs += 4;
			continue;
		}

		if (!(je16_to_cpu(node->nodetype) & JFFS3_NODE_ACCURATE)) {
			/* Wheee. This is an obsoleted node */
			DBG_SCAN(2, "Node at %#08x is obsolete. Skipping\n", ofs);
			if (dirty_space(c, jeb, je32_to_cpu(node->totlen)) != 0)
				return -ENOMEM;
			ofs += PAD(je32_to_cpu(node->totlen));
			continue;
		}

		switch(je16_to_cpu(node->nodetype)) {
		case JFFS3_NODETYPE_INODE:
			if (buf_ofs + buf_len < ofs + sizeof(struct jffs3_raw_inode)) {
				buf_len = min_t(uint32_t, buf_size, jeb->offset + c->sector_size - ofs);
				DBG_SCAN(1, "Fewer than %zd bytes (inode node left to end "
						"of buf. Reading %#08x at %#08x\n",
						sizeof(struct jffs3_raw_inode), buf_len, ofs);
				err = jffs3_fill_scan_buf(c, buf, ofs, buf_len);
				if (err)
					return err;
				buf_ofs = ofs;
				node = (void *)buf;
			}
			err = jffs3_scan_inode_node(c, jeb, (void *)node, ofs);
			if (err) return err;
			ofs += PAD(je32_to_cpu(node->totlen));
			break;

		case JFFS3_NODETYPE_DIRENT:
			if (buf_ofs + buf_len < ofs + je32_to_cpu(node->totlen)) {
				buf_len = min_t(uint32_t, buf_size, jeb->offset + c->sector_size - ofs);
				DBG_SCAN(1, "Fewer than %d bytes (dirent node "
						"left to end of buf. Reading %#08x at %#08x\n",
						je32_to_cpu(node->totlen), buf_len, ofs);
				err = jffs3_fill_scan_buf(c, buf, ofs, buf_len);
				if (err)
					return err;
				buf_ofs = ofs;
				node = (void *)buf;
			}
			err = jffs3_scan_dirent_node(c, jeb, (void *)node, ofs);
			if (err) return err;
			ofs += PAD(je32_to_cpu(node->totlen));
			break;

		case JFFS3_NODETYPE_CLEANMARKER:
			DBG_SCAN(1, "CLEANMARKER node found at %#08x\n", ofs);
			if (je32_to_cpu(node->totlen) != c->cleanmarker_size) {
				NOTICE_MSG("CLEANMARKER node found at %#08x has totlen %#08x != normal %#08x\n",
					ofs, je32_to_cpu(node->totlen), c->cleanmarker_size);
				if (dirty_space(c, jeb, sizeof(struct jffs3_unknown_node)) != 0)
					return -ENOMEM;
				ofs += PAD(sizeof(struct jffs3_unknown_node));
			} else {
				struct jffs3_raw_node_ref *marker_ref;

				if (jeb->first_node) {
					WARNING_MSG("CLEANMARKER node found at %#08x, not first node in "
						"block (%#08x)\n", ofs, jeb->offset);
					if (dirty_space(c, jeb, c->cleanmarker_size) != 0)
						return - ENOMEM;
					ofs += PAD(c->cleanmarker_size);
				}

				if ((marker_ref = jffs3_alloc_raw_node_ref()) == NULL) {
					ERROR_MSG("Failed to allocate node ref for clean marker\n");
					return -ENOMEM;
				}
				marker_ref->next_in_ino = NULL;
				marker_ref->next_phys = NULL;
				marker_ref->flash_offset = ofs | REF_NORMAL;
				marker_ref->__totlen = c->cleanmarker_size;
				jeb->first_node = jeb->last_node = marker_ref;
				USED_SPACE(PAD(c->cleanmarker_size));
				ofs += PAD(c->cleanmarker_size);
			}
			break;

		case JFFS3_NODETYPE_PADDING:
#ifdef CONFIG_JFFS3_SUMMARY
			jffs3_sum_add_padding_mem(jeb,je32_to_cpu(node->totlen));
#endif
			DBG_SCAN(2, "Padding node found at %#08x (length %x)\n", ofs, je32_to_cpu(node->totlen));
			if (dirty_space(c, jeb, je32_to_cpu(node->totlen)) != 0)
				return - ENOMEM;
			ofs += PAD(je32_to_cpu(node->totlen));
			break;

		default:
			switch (je16_to_cpu(node->nodetype) & JFFS3_COMPAT_MASK) {
			case JFFS3_FEATURE_ROCOMPAT:
				DBG_SCAN(1, "Read-only compatible feature node (0x%04x) found at "
						"offset %#08x\n", je16_to_cpu(node->nodetype), ofs);
			        c->flags |= JFFS3_SB_FLAG_RO;
				if (!(jffs3_is_readonly(c)))
					return -EROFS;
				if (dirty_space(c, jeb, je32_to_cpu(node->totlen)) != 0)
					return - ENOMEM;
				ofs += PAD(je32_to_cpu(node->totlen));
				break;

			case JFFS3_FEATURE_INCOMPAT:
				DBG_SCAN(1, "Incompatible feature node (0x%04x) found at offset %#08x\n",
						je16_to_cpu(node->nodetype), ofs);
				return -EINVAL;

			case JFFS3_FEATURE_RWCOMPAT_DELETE:
				DBG_SCAN(1, "Unknown but compatible feature node (0x%04x) found at "
						"offset %#08x\n", je16_to_cpu(node->nodetype), ofs);
				if (dirty_space(c, jeb, je32_to_cpu(node->totlen)) != 0)
					return - ENOMEM;
				ofs += PAD(je32_to_cpu(node->totlen));
				break;

			case JFFS3_FEATURE_RWCOMPAT_COPY:
				DBG_SCAN(1, "Unknown but compatible feature node (0x%04x) found at "
						"offset %#08x\n", je16_to_cpu(node->nodetype), ofs);
				USED_SPACE(PAD(je32_to_cpu(node->totlen)));
				ofs += PAD(je32_to_cpu(node->totlen));
				break;
			}
		}
	}

#ifdef CONFIG_JFFS3_SUMMARY
	if (jeb->sum_collected) {
		if (PAD(jeb->sum_collected->sum_size + JFFS3_SUMMARY_FRAME_SIZE) > jeb->free_size) {
			DBG_SUMMARY(1,"There is not enough space for " 
				"summary information, freeing up summary info!\n");
			jffs3_sum_clean_collected(jeb);
			jeb->sum_collected->sum_size = JFFS3_SUMMARY_NOSUM_SIZE;
		}
		
	}
	else {
		DBG_SUMMARY(1,"Empty summary info found\n");
	}
#endif	
	
	DBG_SCAN(1, "Block at %#08x: free %#08x, dirty %#08x, unchecked %#08x, "
			"used %#08x\n", jeb->offset, jeb->free_size, jeb->dirty_size,
			jeb->unchecked_size, jeb->used_size);

	/* mark_node_obsolete can add to wasted !! */
	if (jeb->wasted_size) {
		jeb->dirty_size += jeb->wasted_size;
		c->dirty_size += jeb->wasted_size;
		c->wasted_size -= jeb->wasted_size;
		jeb->wasted_size = 0;
	}

	if ((jeb->used_size + jeb->unchecked_size) == PAD(c->cleanmarker_size) && !jeb->dirty_size
		&& (!jeb->first_node || !jeb->first_node->next_phys) )
		return BLK_STATE_CLEANMARKER;

	/* move blocks with max 4 byte dirty space to cleanlist */
	else if (!ISDIRTY(c->sector_size - (jeb->used_size + jeb->unchecked_size))) {
		c->dirty_size -= jeb->dirty_size;
		c->wasted_size += jeb->dirty_size;
		jeb->wasted_size += jeb->dirty_size;
		jeb->dirty_size = 0;
		return BLK_STATE_CLEAN;
	} else if (jeb->used_size || jeb->unchecked_size)
		return BLK_STATE_PARTDIRTY;
	else
		return BLK_STATE_ALLDIRTY;
}

struct jffs3_inode_cache *jffs3_scan_make_ino_cache(struct jffs3_sb_info *c, uint32_t ino)
{
	struct jffs3_inode_cache *ic;

	ic = jffs3_get_ino_cache(c, ino);
	if (ic)
		return ic;

	if (ino > c->highest_ino)
		c->highest_ino = ino;

	ic = jffs3_alloc_inode_cache();
	if (!ic) {
		ERROR_MSG("Allocation of inode cache failed\n");
		return NULL;
	}
	memset(ic, 0, sizeof(*ic));

	ic->ino = ino;
	ic->nodes = (void *)ic;
	jffs3_add_ino_cache(c, ic);
	if (ino == 1)
		ic->nlink = 1;
	return ic;
}

static int jffs3_scan_inode_node(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb,
				 struct jffs3_raw_inode *ri, uint32_t ofs)
{
	struct jffs3_raw_node_ref *raw;
	struct jffs3_inode_cache *ic;
	uint32_t ino = je32_to_cpu(ri->ino);

	DBG_SCAN(1, "Node at %#08x\n", ofs);

	/* We do very little here now. Just check the ino# to which we should attribute
	   this node; we can do all the CRC checking etc. later. There's a tradeoff here --
	   we used to scan the flash once only, reading everything we want from it into
	   memory, then building all our in-core data structures and freeing the extra
	   information. Now we allow the first part of the mount to complete a lot quicker,
	   but we have to go _back_ to the flash in order to finish the CRC checking, etc.
	   Which means that the _full_ amount of time to get to proper write mode with GC
	   operational may actually be _longer_ than before. Sucks to be me. */

	raw = jffs3_alloc_raw_node_ref();
	if (!raw) {
		ERROR_MSG("Allocation of node reference failed\n");
		return -ENOMEM;
	}

	ic = jffs3_get_ino_cache(c, ino);
	if (!ic) {
		/* Inocache get failed. Either we read a bogus ino# or it's just genuinely the
		   first node we found for this inode. Do a CRC check to protect against the former
		   case */
		uint32_t crc = crc32(0, ri, sizeof(*ri)-8);

		if (crc != je32_to_cpu(ri->node_crc)) {
			NOTICE_MSG("CRC failed on node at %#08x: Read %#08x, calculated %#08x\n",
					ofs, je32_to_cpu(ri->node_crc), crc);
			jffs3_free_raw_node_ref(raw);
			/* We believe totlen because the CRC on the node _header_ was OK, just the node itself failed. */
			if (dirty_space(c, jeb, je32_to_cpu(ri->totlen)) != 0)
				return -ENOMEM;
			return 0;
		}
		ic = jffs3_scan_make_ino_cache(c, ino);
		if (!ic) {
			jffs3_free_raw_node_ref(raw);
			return -ENOMEM;
		}
	}
	
	/* Wheee. It worked */

	raw->flash_offset = ofs | REF_UNCHECKED;
#ifdef TMP_TOTLEN
	raw->__totlen = PAD(je32_to_cpu(ri->totlen));
#endif
	raw->next_phys = NULL;
	raw->next_in_ino = ic->nodes;

	ic->nodes = raw;
	if (!jeb->first_node)
		jeb->first_node = raw;
	if (jeb->last_node)
		jeb->last_node->next_phys = raw;
	jeb->last_node = raw;

	DBG_SCAN(1, "Node is ino #%u, version %d. Range %#08x-%#08x\n",
		  je32_to_cpu(ri->ino), je32_to_cpu(ri->version), je32_to_cpu(ri->offset),
		  je32_to_cpu(ri->offset)+je32_to_cpu(ri->dsize));

	pseudo_random += je32_to_cpu(ri->version);

	UNCHECKED_SPACE(PAD(je32_to_cpu(ri->totlen)));

#ifdef CONFIG_JFFS3_SUMMARY
	jffs3_sum_add_inode_mem(jeb,ri,ofs);
#endif

	return 0;
}

static int jffs3_scan_dirent_node(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb,
				  struct jffs3_raw_dirent *rd, uint32_t ofs)
{
	struct jffs3_raw_node_ref *raw;
	struct jffs3_full_dirent *fd;
	struct jffs3_inode_cache *ic;
	uint32_t crc;

	DBG_SCAN(1, "Node at %#08x\n", ofs);

	/* We don't get here unless the node is still valid, so we don't have to
	   mask in the ACCURATE bit any more. */
	crc = crc32(0, rd, sizeof(*rd)-8);

	if (crc != je32_to_cpu(rd->node_crc)) {
		NOTICE_MSG("Node CRC failed on node at %#08x: Read %#08x, calculated %#08x\n",
			       ofs, je32_to_cpu(rd->node_crc), crc);
		/* We believe totlen because the CRC on the node _header_ was OK, just the node itself failed. */
		if (dirty_space(c, jeb, je32_to_cpu(rd->totlen)) != 0)
			return -ENOMEM;
		return 0;
	}

	pseudo_random += je32_to_cpu(rd->version);

	fd = jffs3_alloc_full_dirent(rd->nsize+1);
	if (!fd) {
		return -ENOMEM;
	}
	memcpy(&fd->name, rd->name, rd->nsize);
	fd->name[rd->nsize] = 0;

	crc = crc32(0, fd->name, rd->nsize);
	if (crc != je32_to_cpu(rd->name_crc)) {
		NOTICE_MSG("jffs3_scan_dirent_node(): Name CRC failed on node at %#08x: Read %#08x, "
				"calculated %#08x\n", ofs, je32_to_cpu(rd->name_crc), crc);
		DBG_SCAN(1, "Name for which CRC failed is (now) '%s', ino #%d\n",
					fd->name, je32_to_cpu(rd->ino));
		jffs3_free_full_dirent(fd);
		/* FIXME: Why do we believe totlen? */
		/* We believe totlen because the CRC on the node _header_ was OK, just the name failed. */
		if (dirty_space(c, jeb, je32_to_cpu(rd->totlen)) != 0)
			return -ENOMEM;
		return 0;
	}
	raw = jffs3_alloc_raw_node_ref();
	if (!raw) {
		jffs3_free_full_dirent(fd);
		ERROR_MSG("Allocation of node reference failed\n");
		return -ENOMEM;
	}
	
	ic = jffs3_scan_make_ino_cache(c, je32_to_cpu(rd->pino));
	if (!ic) {
		jffs3_free_full_dirent(fd);
		jffs3_free_raw_node_ref(raw);
		return -ENOMEM;
	}

#ifdef TMP_TOTLEN
	raw->__totlen = PAD(je32_to_cpu(rd->totlen));
#endif
	raw->flash_offset = ofs | REF_PRISTINE;
	raw->next_phys = NULL;
	raw->next_in_ino = ic->nodes;
	ic->nodes = raw;
	if (!jeb->first_node)
		jeb->first_node = raw;
	if (jeb->last_node)
		jeb->last_node->next_phys = raw;
	jeb->last_node = raw;

	fd->raw = raw;
	fd->next = NULL;
	fd->version = je32_to_cpu(rd->version);
	fd->ino = je32_to_cpu(rd->ino);
	fd->nhash = full_name_hash(fd->name, rd->nsize);
	fd->type = rd->type;
	USED_SPACE(PAD(je32_to_cpu(rd->totlen)));
	jffs3_add_fd_to_list(c, fd, &ic->scan_dents);

#ifdef CONFIG_JFFS3_SUMMARY
	jffs3_sum_add_dirent_mem(jeb,rd,ofs);
#endif

	return 0;
}

static int count_list(struct list_head *l)
{
	uint32_t count = 0;
	struct list_head *tmp;

	list_for_each(tmp, l) {
		count++;
	}
	return count;
}

/* Note: This breaks if list_empty(head). I don't care. You
   might, if you copy this code and use it elsewhere :) */
static void rotate_list(struct list_head *head, uint32_t count)
{
	struct list_head *n = head->next;

	list_del(head);
	while(count--) {
		n = n->next;
	}
	list_add(head, n);
}

void jffs3_rotate_lists(struct jffs3_sb_info *c)
{
	uint32_t x;
	uint32_t rotateby;

	x = count_list(&c->clean_list);
	if (x) {
		rotateby = pseudo_random % x;
		DBG_SCAN(1, "Rotating clean_list by %d\n", rotateby);

		rotate_list((&c->clean_list), rotateby);

		DBG_SCAN(1, "Erase block at front of clean_list is at %#08x\n",
			  list_entry(c->clean_list.next, struct jffs3_eraseblock, list)->offset);
	} else {
		DBG_SCAN(1, "Not rotating empty clean_list\n");
	}

	x = count_list(&c->very_dirty_list);
	if (x) {
		rotateby = pseudo_random % x;
		DBG_SCAN(1, "Rotating very_dirty_list by %d\n", rotateby);

		rotate_list((&c->very_dirty_list), rotateby);

		DBG_SCAN(1, "Erase block at front of very_dirty_list is at %#08x\n",
			  list_entry(c->very_dirty_list.next, struct jffs3_eraseblock, list)->offset);
	} else {
		DBG_SCAN(1, "Not rotating empty very_dirty_list\n");
	}

	x = count_list(&c->dirty_list);
	if (x) {
		rotateby = pseudo_random % x;
		DBG_SCAN(1, "Rotating dirty_list by %d\n", rotateby);

		rotate_list((&c->dirty_list), rotateby);

		DBG_SCAN(1, "Erase block at front of dirty_list is at %#08x\n",
			  list_entry(c->dirty_list.next, struct jffs3_eraseblock, list)->offset);
	} else {
		DBG_SCAN(1, "Not rotating empty dirty_list\n");
	}

	x = count_list(&c->erasable_list);
	if (x) {
		rotateby = pseudo_random % x;
		DBG_SCAN(1, "Rotating erasable_list by %d\n", rotateby);

		rotate_list((&c->erasable_list), rotateby);

		DBG_SCAN(1, "Erase block at front of erasable_list is at %#08x\n",
			  list_entry(c->erasable_list.next, struct jffs3_eraseblock, list)->offset);
	} else {
		DBG_SCAN(1, "Not rotating empty erasable_list\n");
	}

	if (c->nr_erasing_blocks) {
		rotateby = pseudo_random % c->nr_erasing_blocks;
		DBG_SCAN(1, "Rotating erase_pending_list by %d\n", rotateby);

		rotate_list((&c->erase_pending_list), rotateby);

		DBG_SCAN(1, "Erase block at front of erase_pending_list is at %#08x\n",
			  list_entry(c->erase_pending_list.next, struct jffs3_eraseblock, list)->offset);
	} else {
		DBG_SCAN(1, "Not rotating empty erase_pending_list\n");
	}

	if (c->nr_free_blocks) {
		rotateby = pseudo_random % c->nr_free_blocks;
		DBG_SCAN(1, "Rotating free_list by %d\n", rotateby);

		rotate_list((&c->free_list), rotateby);

		DBG_SCAN(1, "Erase block at front of free_list is at %#08x\n",
			  list_entry(c->free_list.next, struct jffs3_eraseblock, list)->offset);
	} else {
		DBG_SCAN(1, "Not rotating empty free_list\n");
	}
}

/*
 * JFFS3 -- Journalling Flash File System, Version 3.
 *
 * Copyright (C) 2001-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@infradead.org>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * JFFS2 Id: erase.c,v 1.69 2004/12/06 10:17:48 dedekind  Exp
 * $Id: erase.c,v 3.10 2005/02/09 14:52:05 pavlov Exp $
 *
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mtd/mtd.h>
#include <linux/compiler.h>
#include <linux/crc32.h>
#include <linux/sched.h>
#include <linux/pagemap.h>
#include "nodelist.h"
#include "summary.h"

struct erase_priv_struct {
	struct jffs3_eraseblock *jeb;
	struct jffs3_sb_info *c;
};

#ifndef __ECOS
static void jffs3_erase_callback(struct erase_info *);
#endif
static void jffs3_erase_failed(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb, uint32_t bad_offset);
static void jffs3_erase_succeeded(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb);
static void jffs3_free_all_node_refs(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb);
static void jffs3_mark_erased_block(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb);

void jffs3_erase_block(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb)
{
	int ret;
	uint32_t bad_offset;
#ifdef __ECOS
       ret = jffs3_flash_erase(c, jeb);
       if (!ret) {
               jffs3_erase_succeeded(c, jeb);
               return;
       }
       bad_offset = jeb->offset;
#else /* Linux */
	struct erase_info *instr;

	DBG_ERASE(1, "erase block %#x (range %#x-%#x)\n", jeb->offset, jeb->offset,
			jeb->offset + c->sector_size);
	instr = kmalloc(sizeof(struct erase_info) + sizeof(struct erase_priv_struct), GFP_KERNEL);
	if (!instr) {
		WARNING_MSG("kmalloc for struct erase_info in jffs3_erase_block failed. "
				"Refiling block for later\n");
		spin_lock(&c->erase_completion_lock);
		list_del(&jeb->list);
		list_add(&jeb->list, &c->erase_pending_list);
		c->erasing_size -= c->sector_size;
		c->dirty_size += c->sector_size;
		jeb->dirty_size = c->sector_size;
		spin_unlock(&c->erase_completion_lock);
		return;
	}

	memset(instr, 0, sizeof(*instr));

	instr->mtd = c->mtd;
	instr->addr = jeb->offset;
	instr->len = c->sector_size;
	instr->callback = jffs3_erase_callback;
	instr->priv = (unsigned long)(&instr[1]);
	instr->fail_addr = 0xffffffff;

	((struct erase_priv_struct *)instr->priv)->jeb = jeb;
	((struct erase_priv_struct *)instr->priv)->c = c;

	ret = c->mtd->erase(c->mtd, instr);
	if (!ret)
		return;

	bad_offset = instr->fail_addr;
	kfree(instr);
#endif /* __ECOS */

	if (ret == -ENOMEM || ret == -EAGAIN) {
		/* Erase failed immediately. Refile it on the list */
		DBG_ERASE(1 ,"Erase at %#08x failed: %d. Refiling on erase_pending_list\n", jeb->offset, ret);
		spin_lock(&c->erase_completion_lock);
		list_del(&jeb->list);
		list_add(&jeb->list, &c->erase_pending_list);
		c->erasing_size -= c->sector_size;
		c->dirty_size += c->sector_size;
		jeb->dirty_size = c->sector_size;
		spin_unlock(&c->erase_completion_lock);
		return;
	}

	if (ret == -EROFS)
		WARNING_MSG("Erase at %#08x failed immediately: -EROFS. Is the sector locked?\n", jeb->offset);
	else
		WARNING_MSG("Erase at %#08x failed immediately: errno %d\n", jeb->offset, ret);

	jffs3_erase_failed(c, jeb, bad_offset);
}

void jffs3_erase_pending_blocks(struct jffs3_sb_info *c, int count)
{
	struct jffs3_eraseblock *jeb;

	down(&c->erase_free_sem);

	spin_lock(&c->erase_completion_lock);

	while (!list_empty(&c->erase_complete_list) ||
	       !list_empty(&c->erase_pending_list)) {

		if (!list_empty(&c->erase_complete_list)) {
			jeb = list_entry(c->erase_complete_list.next, struct jffs3_eraseblock, list);
			list_del(&jeb->list);
			spin_unlock(&c->erase_completion_lock);
			jffs3_mark_erased_block(c, jeb);

			if (!--count) {
				DBG_ERASE(1, "Count reached, leaving\n");
				goto done;
			}

		} else if (!list_empty(&c->erase_pending_list)) {
			jeb = list_entry(c->erase_pending_list.next, struct jffs3_eraseblock, list);
			DBG_ERASE(1, "Starting erase of pending block %#08x\n", jeb->offset);
			list_del(&jeb->list);
			c->erasing_size += c->sector_size;
			c->wasted_size -= jeb->wasted_size;
			c->free_size -= jeb->free_size;
			c->used_size -= jeb->used_size;
			c->dirty_size -= jeb->dirty_size;
			jeb->wasted_size = jeb->used_size = jeb->dirty_size = jeb->free_size = 0;
			jffs3_free_all_node_refs(c, jeb);
			list_add(&jeb->list, &c->erasing_list);
			spin_unlock(&c->erase_completion_lock);

			jffs3_erase_block(c, jeb);

		} else {
			BUG();
		}

		/* Be nice */
		cond_resched();
		spin_lock(&c->erase_completion_lock);
	}

	spin_unlock(&c->erase_completion_lock);
 done:
	DBG_ERASE(1, "completed\n");

	up(&c->erase_free_sem);
}

static void jffs3_erase_succeeded(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb)
{
	DBG_ERASE(1, "Erase completed successfully at %#08x\n", jeb->offset);
	spin_lock(&c->erase_completion_lock);
	list_del(&jeb->list);
	list_add_tail(&jeb->list, &c->erase_complete_list);
	spin_unlock(&c->erase_completion_lock);
	/* Ensure that kupdated calls us again to mark them clean */
	jffs3_erase_pending_trigger(c);
}

static void jffs3_erase_failed(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb, uint32_t bad_offset)
{
	/* For NAND, if the failure did not occur at the device level for a
	   specific physical page, don't bother updating the bad block table. */
	if (jffs3_cleanmarker_oob(c) && (bad_offset != 0xffffffff)) {
		/* We had a device-level failure to erase.  Let's see if we've
		   failed too many times. */
		if (!jffs3_write_nand_badblock(c, jeb, bad_offset)) {
			/* We'd like to give this block another try. */
			spin_lock(&c->erase_completion_lock);
			list_del(&jeb->list);
			list_add(&jeb->list, &c->erase_pending_list);
			c->erasing_size -= c->sector_size;
			c->dirty_size += c->sector_size;
			jeb->dirty_size = c->sector_size;
			spin_unlock(&c->erase_completion_lock);
			return;
		}
	}

	spin_lock(&c->erase_completion_lock);
	c->erasing_size -= c->sector_size;
	c->bad_size += c->sector_size;
	list_del(&jeb->list);
	list_add(&jeb->list, &c->bad_list);
	c->nr_erasing_blocks--;
	spin_unlock(&c->erase_completion_lock);
	wake_up(&c->erase_wait);
}

#ifndef __ECOS
static void jffs3_erase_callback(struct erase_info *instr)
{
	struct erase_priv_struct *priv = (void *)instr->priv;

	if(instr->state != MTD_ERASE_DONE) {
		WARNING_MSG("Erase at %#08x finished, but state != MTD_ERASE_DONE. "
				"State is 0x%x instead.\n", instr->addr, instr->state);
		jffs3_erase_failed(priv->c, priv->jeb, instr->fail_addr);
	} else {
		jffs3_erase_succeeded(priv->c, priv->jeb);
	}
	kfree(instr);
}
#endif /* !__ECOS */

/* Hmmm. Maybe we should accept the extra space it takes and make
   this a standard doubly-linked list? */
static inline void jffs3_remove_node_refs_from_ino_list(struct jffs3_sb_info *c,
			struct jffs3_raw_node_ref *ref, struct jffs3_eraseblock *jeb)
{
	struct jffs3_inode_cache *ic = NULL;
	struct jffs3_raw_node_ref **prev;

	prev = &ref->next_in_ino;

	/* Walk the inode's list once, removing any nodes from this eraseblock */
	while (1) {
		if (!(*prev)->next_in_ino) {
			/* We're looking at the jffs3_inode_cache, which is
			   at the end of the linked list. Stash it and continue
			   from the beginning of the list */
			ic = (struct jffs3_inode_cache *)(*prev);
			prev = &ic->nodes;
			continue;
		}

		if (SECTOR_ADDR((*prev)->flash_offset) == jeb->offset) {
			/* It's in the block we're erasing */
			struct jffs3_raw_node_ref *this;

			this = *prev;
			*prev = this->next_in_ino;
			this->next_in_ino = NULL;

			if (this == ref)
				break;

			continue;
		}
		/* Not to be deleted. Skip */
		prev = &((*prev)->next_in_ino);
	}

	/* PARANOIA */
	if (!ic) {
		WARNING_MSG("inode_cache not found!!\n");
		return;
	}

	DBG_NR(1, "Removed nodes in range %#08x-%#08x from ino #%u\n",
		  jeb->offset, jeb->offset + c->sector_size, ic->ino);

	if (ic->nodes == (void *)ic) {
		DBG_NR(1, "inocache for ino #%u is all gone now. Freeing\n", ic->ino);
		jffs3_del_ino_cache(c, ic);
		jffs3_free_inode_cache(ic);
	}
}

static void jffs3_free_all_node_refs(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb)
{
	struct jffs3_raw_node_ref *ref;
	DBG_NR(1, "Freeing all node refs for eraseblock offset %#08x\n", jeb->offset);
	while(jeb->first_node) {
		ref = jeb->first_node;
		jeb->first_node = ref->next_phys;

		/* Remove from the inode-list */
		if (ref->next_in_ino)
			jffs3_remove_node_refs_from_ino_list(c, ref, jeb);
		/* else it was a non-inode node or already removed, so don't bother */

		DBG_ERASE(2, "Free ref at %#08x(%#x), len %x\n",
			ref_offset(ref), ref_flags(ref), ref_totlen(c, jeb, ref));
		jffs3_free_raw_node_ref(ref);
	}
	jeb->last_node = NULL;
}

static void jffs3_mark_erased_block(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb)
{
	struct jffs3_raw_node_ref *marker_ref = NULL;
	unsigned char *ebuf;
	size_t retlen;
	int ret;
	uint32_t bad_offset;

	if (!jffs3_cleanmarker_oob(c)) {
		marker_ref = jffs3_alloc_raw_node_ref();
		if (!marker_ref) {
			WARNING_MSG("Failed to allocate raw node ref for clean marker\n");
			/* Stick it back on the list from whence it came and come back later */
			jffs3_erase_pending_trigger(c);
			spin_lock(&c->erase_completion_lock);
			list_add(&jeb->list, &c->erase_complete_list);
			spin_unlock(&c->erase_completion_lock);
			return;
		}
	}
	ebuf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!ebuf) {
		WARNING_MSG("Failed to allocate page buffer for verifying erase "
				"at %#08x. Assuming it worked\n", jeb->offset);
	} else {
		uint32_t ofs = jeb->offset;

		DBG_ERASE(1, "Verifying erase at %#08x\n", jeb->offset);
		while(ofs < jeb->offset + c->sector_size) {
			uint32_t readlen = min((uint32_t)PAGE_SIZE, jeb->offset + c->sector_size - ofs);
			int i;

			bad_offset = ofs;

			ret = jffs3_flash_read(c, ofs, readlen, &retlen, ebuf);
			if (ret) {
				WARNING_MSG("Read of newly-erased block at %#08x failed: %d. "
						"Putting on bad_list\n", ofs, ret);
				goto bad;
			}
			if (retlen != readlen) {
				WARNING_MSG("Short read from newly-erased block at %#08x. "
						"Wanted %d, got %zd\n", ofs, readlen, retlen);
				goto bad;
			}
			for (i=0; i<readlen; i += sizeof(unsigned long)) {
				/* It's OK. We know it's properly aligned */
				unsigned long datum = *(unsigned long *)(&ebuf[i]);
				if (datum + 1) {
					bad_offset += i;
					WARNING_MSG("Newly-erased block contained word 0x%lx "
							"at offset %#08x\n", datum, bad_offset);
				bad:
					if (!jffs3_cleanmarker_oob(c))
						jffs3_free_raw_node_ref(marker_ref);
					kfree(ebuf);
				bad2:
					spin_lock(&c->erase_completion_lock);
					/* Stick it on a list (any list) so
					   erase_failed can take it right off
					   again.  Silly, but shouldn't happen
					   often. */
					list_add(&jeb->list, &c->erasing_list);
					spin_unlock(&c->erase_completion_lock);
					jffs3_erase_failed(c, jeb, bad_offset);
					return;
				}
			}
			ofs += readlen;
			cond_resched();
		}
		kfree(ebuf);
	}

	bad_offset = jeb->offset;

	/* Write the erase complete marker */
	DBG_ERASE(1, "Writing erased marker to block at %#08x\n", jeb->offset);
	if (jffs3_cleanmarker_oob(c)) {

		if (jffs3_write_nand_cleanmarker(c, jeb))
			goto bad2;

		jeb->first_node = jeb->last_node = NULL;

		jeb->free_size = c->sector_size;
		jeb->used_size = 0;
		jeb->dirty_size = 0;
		jeb->wasted_size = 0;
	} else {
		struct kvec vecs[1];
		struct jffs3_unknown_node marker = {
			.magic =	cpu_to_je16(JFFS3_MAGIC_BITMASK),
			.nodetype =	cpu_to_je16(JFFS3_NODETYPE_CLEANMARKER),
			.totlen =	cpu_to_je32(c->cleanmarker_size)
		};

		marker.hdr_crc = cpu_to_je32(crc32(0, &marker, sizeof(struct jffs3_unknown_node)-4));

		vecs[0].iov_base = (unsigned char *) &marker;
		vecs[0].iov_len = sizeof(marker);
		ret = jffs3_flash_direct_writev(c, vecs, 1, jeb->offset, &retlen);

		if (ret) {
			WARNING_MSG("Write clean marker to block at %#08x failed: %d\n",
				       jeb->offset, ret);
			goto bad2;
		}
		if (retlen != sizeof(marker)) {
			WARNING_MSG("Short write to newly-erased block at %#08x: Wanted %zd, got %zd\n",
				       jeb->offset, sizeof(marker), retlen);
			goto bad2;
		}

		marker_ref->next_in_ino = NULL;
		marker_ref->next_phys = NULL;
		marker_ref->flash_offset = jeb->offset | REF_NORMAL;
#ifdef TMP_TOTLEN
		marker_ref->__totlen = c->cleanmarker_size;
#endif

		jeb->first_node = jeb->last_node = marker_ref;

		jeb->free_size = c->sector_size - c->cleanmarker_size;
		jeb->used_size = c->cleanmarker_size;
		jeb->dirty_size = 0;
		jeb->wasted_size = 0;
	}

#ifdef CONFIG_JFFS3_SUMMARY
	
	if (jeb->sum_collected) {
	
		jffs3_sum_clean_collected(jeb);
		jeb->sum_collected->sum_size = 0;
		jeb->sum_collected->sum_padded = 0;
	}
#endif
	
	spin_lock(&c->erase_completion_lock);
	c->erasing_size -= c->sector_size;
	c->free_size += jeb->free_size;
	c->used_size += jeb->used_size;

	jffs3_dbg_acct_sanity_check(c, jeb);
	if (DEBUG1)
		jffs3_dbg_acct_paranoia_check(c, jeb);

	list_add_tail(&jeb->list, &c->free_list);
	c->nr_erasing_blocks--;
	c->nr_free_blocks++;
	spin_unlock(&c->erase_completion_lock);
	wake_up(&c->erase_wait);
}

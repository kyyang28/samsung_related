/*
 * JFFS2 -- Journalling Flash File System, Version 2.
 *
 * Copyright (C) 2001-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@infradead.org>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * $Id: debug.c,v 1.6 2005/07/22 10:32:07 dedekind Exp $
 *
 */
#include <linux/kernel.h>
#include <linux/pagemap.h>
#include "nodelist.h"
#include "debug.h"

#ifdef JFFS2_DBG_PARANOIA_CHECKS
/*
 * Check the fragtree.
 */
void
__jffs2_dbg_fragtree_paranoia_check(struct jffs2_inode_info *f)
{
	down(&f->sem);
	__jffs2_dbg_fragtree_paranoia_check_nolock(f);
	up(&f->sem);
}
	
void
__jffs2_dbg_fragtree_paranoia_check_nolock(struct jffs2_inode_info *f)
{
	struct jffs2_node_frag *frag;
	int bitched = 0;

	for (frag = frag_first(&f->fragtree); frag; frag = frag_next(frag)) {
		struct jffs2_full_dnode *fn = frag->node;

		if (!fn || !fn->raw)
			continue;

		if (ref_flags(fn->raw) == REF_PRISTINE) {
			if (fn->frags > 1) {
				printk(KERN_ERR "REF_PRISTINE node at 0x%08x had %d frags. Tell dwmw2\n",
						ref_offset(fn->raw), fn->frags);
				bitched = 1;
			}

			/* A hole node which isn't multi-page should be garbage-collected
			   and merged anyway, so we just check for the frag size here,
			   rather than mucking around with actually reading the node
			   and checking the compression type, which is the real way
			   to tell a hole node. */
			if (frag->ofs & (PAGE_CACHE_SIZE-1) && frag_prev(frag)
					&& frag_prev(frag)->size < PAGE_CACHE_SIZE && frag_prev(frag)->node) {
				printk(KERN_ERR "REF_PRISTINE node at 0x%08x had a previous non-hole frag "
						"in the same page. Tell dwmw2\n", ref_offset(fn->raw));
				bitched = 1;
			}

			if ((frag->ofs+frag->size) & (PAGE_CACHE_SIZE-1) && frag_next(frag)
					&& frag_next(frag)->size < PAGE_CACHE_SIZE && frag_next(frag)->node) {
				printk(KERN_ERR "REF_PRISTINE node at 0x%08x (%08x-%08x) had a following "
						"non-hole frag in the same page. Tell dwmw2\n",
					       ref_offset(fn->raw), frag->ofs, frag->ofs+frag->size);
				bitched = 1;
			}
		}
	}

	if (bitched) {
		printk(KERN_ERR "Fragtree is corrupted. Fragtree dump:\n");
		__jffs2_dbg_dump_fragtree_nolock(f);
		BUG();
	}
}

/*
 * Check if the flash contains all 0xFF before we start writing.
 */
void
__jffs2_dbg_prewrite_paranoia_check(struct jffs2_sb_info *c,
				    uint32_t ofs, int len)
{
	size_t retlen;
	int ret, i;
	unsigned char *buf;

	buf = kmalloc(len, GFP_KERNEL);
	if (!buf)
		return;

	ret = jffs2_flash_read(c, ofs, len, &retlen, buf);
	if (ret || (retlen != len)) {
		printk(KERN_WARNING "read %d bytes failed or short in %s(). ret %d, retlen %zd\n",
				len, __FUNCTION__, ret, retlen);
		kfree(buf);
		return;
	}

	ret = 0;
	for (i = 0; i < len; i++)
		if (buf[i] != 0xff)
			ret = 1;

	if (ret) {
		printk(KERN_ERR "ARGH. About to write node to %#08x on flash, but there are data "
				"already there. The first corrupted byte is at %#08x offset.\n", ofs, ofs + i);
		__jffs2_dbg_dump_buffer(buf, len, ofs);
		kfree(buf);
		BUG();
	}

	kfree(buf);
}

/*
 * Check the space accounting and node_ref list correctness for the JFFS2 erasable block 'jeb'.
 */
void
__jffs2_dbg_acct_paranoia_check(struct jffs2_sb_info *c,
				struct jffs2_eraseblock *jeb)
{
	spin_lock(&c->erase_completion_lock);
	__jffs2_dbg_acct_paranoia_check_nolock(c, jeb);
	spin_unlock(&c->erase_completion_lock);
}
	
void
__jffs2_dbg_acct_paranoia_check_nolock(struct jffs2_sb_info *c,
				       struct jffs2_eraseblock *jeb)
{
	uint32_t my_used_size = 0;
	uint32_t my_unchecked_size = 0;
	uint32_t my_dirty_size = 0;
	struct jffs2_raw_node_ref *ref2 = jeb->first_node;

	while (ref2) {
		uint32_t totlen = ref_totlen(c, jeb, ref2);

		if (ref2->flash_offset < jeb->offset ||
				ref2->flash_offset > jeb->offset + c->sector_size) {
			printk(KERN_ERR "node_ref %#08x shouldn't be in block at %#08x!\n",
				ref_offset(ref2), jeb->offset);
			goto error;

		}
		if (ref_flags(ref2) == REF_UNCHECKED)
			my_unchecked_size += totlen;
		else if (!ref_obsolete(ref2))
			my_used_size += totlen;
		else
			my_dirty_size += totlen;

		if ((!ref2->next_phys) != (ref2 == jeb->last_node)) {
			printk(KERN_ERR "node_ref for node at %#08x (mem %p) has next_phys at %#08x (mem %p), "
					"last_node is at %#08x (mem %p)\n",
					ref_offset(ref2), ref2, ref_offset(ref2->next_phys), ref2->next_phys,
					ref_offset(jeb->last_node), jeb->last_node);
			goto error;
		}
		ref2 = ref2->next_phys;
	}

	if (my_used_size != jeb->used_size) {
		printk(KERN_ERR "Calculated used size %#08x != stored used size %#08x\n",
				my_used_size, jeb->used_size);
		goto error;
	}

	if (my_unchecked_size != jeb->unchecked_size) {
		printk(KERN_ERR "Calculated unchecked size %#08x != stored unchecked size %#08x\n",
				my_unchecked_size, jeb->unchecked_size);
		goto error;
	}

#if 0
	/* This should work when we implement ref->__totlen elemination */
	if (my_dirty_size != jeb->dirty_size + jeb->wasted_size) {
		printk(KERN_ERR "Calculated dirty+wasted size %#08x != stored dirty + wasted size %#08x\n",
			my_dirty_size, jeb->dirty_size + jeb->wasted_size);
		goto error;
	}

	if (jeb->free_size == 0
		&& my_used_size + my_unchecked_size + my_dirty_size != c->sector_size) {
		printk(KERN_ERR "The sum of all nodes in block (%#x) != size of block (%#x)\n",
			my_used_size + my_unchecked_size + my_dirty_size,
			c->sector_size);
		goto error;
	}
#endif

	return;

error:
	__jffs2_dbg_dump_node_refs_nolock(c, jeb);
	__jffs2_dbg_dump_jeb_nolock(jeb);
	__jffs2_dbg_dump_block_lists_nolock(c);
	BUG();
	
}
#endif /* JFFS2_DBG_PARANOIA_CHECKS */

#if defined(JFFS2_DBG_DUMPS) || defined(JFFS2_DBG_PARANOIA_CHECKS)
/*
 * Dump the node_refs of the 'jeb' JFFS2 eraseblock.
 */
void
__jffs2_dbg_dump_node_refs(struct jffs2_sb_info *c,
			   struct jffs2_eraseblock *jeb)
{
	spin_lock(&c->erase_completion_lock);
	__jffs2_dbg_dump_node_refs_nolock(c, jeb);
	spin_unlock(&c->erase_completion_lock);
}

void
__jffs2_dbg_dump_node_refs_nolock(struct jffs2_sb_info *c,
				  struct jffs2_eraseblock *jeb)
{
	struct jffs2_raw_node_ref *ref;
	int i = 0;

	printk(KERN_DEBUG "Dump the node_ref oth the eraseblock %#08x\n", jeb->offset);
	if (!jeb->first_node) {
		printk(KERN_DEBUG "no nodes in block %#08x\n", jeb->offset);
		return;
	}

	printk(KERN_DEBUG);
	for (ref = jeb->first_node; ; ref = ref->next_phys) {
		printk("%#08x(%#x)", ref_offset(ref), ref->__totlen);
		if (ref->next_phys)
			printk("->");
		else
			break;
		if (++i == 4) {
			i = 0;
			printk("\n" KERN_DEBUG);
		}
	}
	printk("\n");
}

/*
 * Dump an eraseblock's space accounting.
 */
void
__jffs2_dbg_dump_jeb(struct jffs2_sb_info *c, struct jffs2_eraseblock *jeb)
{
	spin_lock(&c->erase_completion_lock);
	__jffs2_dbg_dump_jeb_nolock(jeb);
	spin_unlock(&c->erase_completion_lock);
}

void
__jffs2_dbg_dump_jeb_nolock(struct jffs2_eraseblock *jeb)
{
	if (!jeb)
		return;

	printk(KERN_DEBUG "dump space accounting for the eraseblock at %#08x\n",
			jeb->offset);

	printk(KERN_DEBUG "used_size: %#08x\n",		jeb->used_size);
	printk(KERN_DEBUG "dirty_size: %#08x\n",	jeb->dirty_size);
	printk(KERN_DEBUG "wasted_size: %#08x\n",	jeb->wasted_size);
	printk(KERN_DEBUG "unchecked_size: %#08x\n",	jeb->unchecked_size);
	printk(KERN_DEBUG "free_size: %#08x\n",		jeb->free_size);
}

void
__jffs2_dbg_dump_block_lists(struct jffs2_sb_info *c)
{
	spin_lock(&c->erase_completion_lock);
	__jffs2_dbg_dump_block_lists_nolock(c);
	spin_unlock(&c->erase_completion_lock);
}

void
__jffs2_dbg_dump_block_lists_nolock(struct jffs2_sb_info *c)
{
	printk(KERN_DEBUG "Dump the blocks list\n");
	printk(KERN_DEBUG "flash_size: %#08x\n",	c->flash_size);
	printk(KERN_DEBUG "used_size: %#08x\n",		c->used_size);
	printk(KERN_DEBUG "dirty_size: %#08x\n",	c->dirty_size);
	printk(KERN_DEBUG "wasted_size: %#08x\n",	c->wasted_size);
	printk(KERN_DEBUG "unchecked_size: %#08x\n",	c->unchecked_size);
	printk(KERN_DEBUG "free_size: %#08x\n",		c->free_size);
	printk(KERN_DEBUG "erasing_size: %#08x\n",	c->erasing_size);
	printk(KERN_DEBUG "bad_size: %#08x\n",		c->bad_size);
	printk(KERN_DEBUG "sector_size: %#08x\n",	c->sector_size);
	printk(KERN_DEBUG "jffs2_reserved_blocks size: %#08x\n",
				c->sector_size * c->resv_blocks_write);

	if (c->nextblock)
		printk(KERN_DEBUG "nextblock: %#08x (used %#08x, dirty %#08x, wasted %#08x, "
				"unchecked %#08x, free %#08x)\n",
				c->nextblock->offset, c->nextblock->used_size,
				c->nextblock->dirty_size, c->nextblock->wasted_size,
				c->nextblock->unchecked_size, c->nextblock->free_size);
	else
		printk(KERN_DEBUG "nextblock: NULL\n");

	if (c->gcblock)
		printk(KERN_DEBUG "gcblock: %#08x (used %#08x, dirty %#08x, wasted %#08x, "
				"unchecked %#08x, free %#08x)\n",
				c->gcblock->offset, c->gcblock->used_size, c->gcblock->dirty_size,
				c->gcblock->wasted_size, c->gcblock->unchecked_size, c->gcblock->free_size);
	else
		printk(KERN_DEBUG "gcblock: NULL\n");

	if (list_empty(&c->clean_list)) {
		printk(KERN_DEBUG "clean_list: empty\n");
	} else {
		struct list_head *this;
		int numblocks = 0;
		uint32_t dirty = 0;

		list_for_each(this, &c->clean_list) {
			struct jffs2_eraseblock *jeb = list_entry(this, struct jffs2_eraseblock, list);
			numblocks ++;
			dirty += jeb->wasted_size;
			if (!(jeb->used_size == 0 && jeb->dirty_size == 0 && jeb->wasted_size == 0)) {
				printk(KERN_DEBUG "clean_list: %#08x (used %#08x, dirty %#08x, wasted %#08x, "
						"unchecked %#08x, free %#08x)\n",
						jeb->offset, jeb->used_size, jeb->dirty_size, jeb->wasted_size,
						jeb->unchecked_size, jeb->free_size);
			}
		}

		printk (KERN_DEBUG "Contains %d blocks with total wasted size %u, average wasted size: %u\n",
				numblocks, dirty, dirty / numblocks);
	}

	if (list_empty(&c->very_dirty_list)) {
		printk(KERN_DEBUG "very_dirty_list: empty\n");
	} else {
		struct list_head *this;
		int numblocks = 0;
		uint32_t dirty = 0;

		list_for_each(this, &c->very_dirty_list) {
			struct jffs2_eraseblock *jeb = list_entry(this, struct jffs2_eraseblock, list);

			numblocks ++;
			dirty += jeb->dirty_size;
			if (!(jeb->used_size == 0 && jeb->dirty_size == 0 && jeb->wasted_size == 0)) {
				printk(KERN_DEBUG "very_dirty_list: %#08x (used %#08x, dirty %#08x, wasted %#08x, "
						"unchecked %#08x, free %#08x)\n",
						jeb->offset, jeb->used_size, jeb->dirty_size, jeb->wasted_size,
						jeb->unchecked_size, jeb->free_size);
			}
		}

		printk (KERN_DEBUG "Contains %d blocks with total dirty size %u, average dirty size: %u\n",
				numblocks, dirty, dirty / numblocks);
	}

	if (list_empty(&c->dirty_list)) {
		printk(KERN_DEBUG "dirty_list: empty\n");
	} else {
		struct list_head *this;
		int numblocks = 0;
		uint32_t dirty = 0;

		list_for_each(this, &c->dirty_list) {
			struct jffs2_eraseblock *jeb = list_entry(this, struct jffs2_eraseblock, list);

			numblocks ++;
			dirty += jeb->dirty_size;
			if (!(jeb->used_size == 0 && jeb->dirty_size == 0 && jeb->wasted_size == 0)) {
				printk(KERN_DEBUG "dirty_list: %#08x (used %#08x, dirty %#08x, wasted %#08x, "
						"unchecked %#08x, free %#08x)\n",
						jeb->offset, jeb->used_size, jeb->dirty_size, jeb->wasted_size,
						jeb->unchecked_size, jeb->free_size);
			}
		}

		printk (KERN_DEBUG "Contains %d blocks with total dirty size %u, average dirty size: %u\n",
			numblocks, dirty, dirty / numblocks);
	}

	if (list_empty(&c->erasable_list)) {
		printk(KERN_DEBUG "erasable_list: empty\n");
	} else {
		struct list_head *this;

		list_for_each(this, &c->erasable_list) {
			struct jffs2_eraseblock *jeb = list_entry(this, struct jffs2_eraseblock, list);

			if (!(jeb->used_size == 0 && jeb->dirty_size == 0 && jeb->wasted_size == 0)) {
				printk(KERN_DEBUG "erasable_list: %#08x (used %#08x, dirty %#08x, wasted %#08x, "
						"unchecked %#08x, free %#08x)\n",
						jeb->offset, jeb->used_size, jeb->dirty_size, jeb->wasted_size,
						jeb->unchecked_size, jeb->free_size);
			}
		}
	}

	if (list_empty(&c->erasing_list)) {
		printk(KERN_DEBUG "erasing_list: empty\n");
	} else {
		struct list_head *this;

		list_for_each(this, &c->erasing_list) {
			struct jffs2_eraseblock *jeb = list_entry(this, struct jffs2_eraseblock, list);

			if (!(jeb->used_size == 0 && jeb->dirty_size == 0 && jeb->wasted_size == 0)) {
				printk(KERN_DEBUG "erasing_list: %#08x (used %#08x, dirty %#08x, wasted %#08x, "
						"unchecked %#08x, free %#08x)\n",
						jeb->offset, jeb->used_size, jeb->dirty_size, jeb->wasted_size,
						jeb->unchecked_size, jeb->free_size);
			}
		}
	}

	if (list_empty(&c->erase_pending_list)) {
		printk(KERN_DEBUG "erase_pending_list: empty\n");
	} else {
		struct list_head *this;

		list_for_each(this, &c->erase_pending_list) {
			struct jffs2_eraseblock *jeb = list_entry(this, struct jffs2_eraseblock, list);

			if (!(jeb->used_size == 0 && jeb->dirty_size == 0 && jeb->wasted_size == 0)) {
				printk(KERN_DEBUG "erase_pending_list: %#08x (used %#08x, dirty %#08x, wasted %#08x, "
						"unchecked %#08x, free %#08x)\n",
						jeb->offset, jeb->used_size, jeb->dirty_size, jeb->wasted_size,
						jeb->unchecked_size, jeb->free_size);
			}
		}
	}

	if (list_empty(&c->erasable_pending_wbuf_list)) {
		printk(KERN_DEBUG "erasable_pending_wbuf_list: empty\n");
	} else {
		struct list_head *this;

		list_for_each(this, &c->erasable_pending_wbuf_list) {
			struct jffs2_eraseblock *jeb = list_entry(this, struct jffs2_eraseblock, list);

			if (!(jeb->used_size == 0 && jeb->dirty_size == 0 && jeb->wasted_size == 0)) {
				printk(KERN_DEBUG "erasable_pending_wbuf_list: %#08x (used %#08x, dirty %#08x, "
						"wasted %#08x, unchecked %#08x, free %#08x)\n",
						jeb->offset, jeb->used_size, jeb->dirty_size, jeb->wasted_size,
						jeb->unchecked_size, jeb->free_size);
			}
		}
	}

	if (list_empty(&c->free_list)) {
		printk(KERN_DEBUG "free_list: empty\n");
	} else {
		struct list_head *this;

		list_for_each(this, &c->free_list) {
			struct jffs2_eraseblock *jeb = list_entry(this, struct jffs2_eraseblock, list);

			if (!(jeb->used_size == 0 && jeb->dirty_size == 0 && jeb->wasted_size == 0)) {
				printk(KERN_DEBUG "free_list: %#08x (used %#08x, dirty %#08x, wasted %#08x, "
						"unchecked %#08x, free %#08x)\n",
						jeb->offset, jeb->used_size, jeb->dirty_size, jeb->wasted_size,
						jeb->unchecked_size, jeb->free_size);
			}
		}
	}

	if (list_empty(&c->bad_list)) {
		printk(KERN_DEBUG "bad_list: empty\n");
	} else {
		struct list_head *this;

		list_for_each(this, &c->bad_list) {
			struct jffs2_eraseblock *jeb = list_entry(this, struct jffs2_eraseblock, list);

			if (!(jeb->used_size == 0 && jeb->dirty_size == 0 && jeb->wasted_size == 0)) {
				printk(KERN_DEBUG "bad_list: %#08x (used %#08x, dirty %#08x, wasted %#08x, "
						"unchecked %#08x, free %#08x)\n",
						jeb->offset, jeb->used_size, jeb->dirty_size, jeb->wasted_size,
						jeb->unchecked_size, jeb->free_size);
			}
		}
	}

	if (list_empty(&c->bad_used_list)) {
		printk(KERN_DEBUG "bad_used_list: empty\n");
	} else {
		struct list_head *this;

		list_for_each(this, &c->bad_used_list) {
			struct jffs2_eraseblock *jeb = list_entry(this, struct jffs2_eraseblock, list);

			if (!(jeb->used_size == 0 && jeb->dirty_size == 0 && jeb->wasted_size == 0)) {
				printk(KERN_DEBUG "bad_used_list: %#08x (used %#08x, dirty %#08x, wasted %#08x, "
						"unchecked %#08x, free %#08x)\n",
						jeb->offset, jeb->used_size, jeb->dirty_size, jeb->wasted_size,
						jeb->unchecked_size, jeb->free_size);
			}
		}
	}
}

void
__jffs2_dbg_dump_fragtree(struct jffs2_inode_info *f)
{
	down(&f->sem);
	jffs2_dbg_dump_fragtree_nolock(f);
	up(&f->sem);
}

void
__jffs2_dbg_dump_fragtree_nolock(struct jffs2_inode_info *f)
{
	struct jffs2_node_frag *this = frag_first(&f->fragtree);
	uint32_t lastofs = 0;
	int buggy = 0;

	printk(KERN_DEBUG "Dump fragtree of ino #%u\n", f->inocache->ino);
	while(this) {
		if (this->node)
			printk(KERN_DEBUG "frag %#04x-%#04x: %#08x(%d) on flash (*%p), left (%p), "
					"right (%p), parent (%p)\n",
					this->ofs, this->ofs+this->size, ref_offset(this->node->raw),
					ref_flags(this->node->raw), this, frag_left(this), frag_right(this),
					frag_parent(this));
		else
			printk(KERN_DEBUG "frag %#04x-%#04x: hole (*%p). left (%p), right (%p), parent (%p)\n",
					this->ofs, this->ofs+this->size, this, frag_left(this),
					frag_right(this), frag_parent(this));
		if (this->ofs != lastofs)
			buggy = 1;
		lastofs = this->ofs + this->size;
		this = frag_next(this);
	}

	if (f->metadata)
		printk(KERN_DEBUG "metadata at 0x%08x\n", ref_offset(f->metadata->raw));

	if (buggy) {
		printk(KERN_ERR "Error! %s(): Frag tree got a hole in it\n", __FUNCTION__);
		BUG();
	}
}

#define JFFS2_BUFDUMP_BYTES_PER_LINE	32
void
__jffs2_dbg_dump_buffer(unsigned char *buf, int len, uint32_t offs)
{
	int skip;
	int i;
	
	printk(KERN_DEBUG "Dump from offset %#08x to offset %#08x (%x bytes)\n",
			offs, offs + len, len);
	i = skip = offs % JFFS2_BUFDUMP_BYTES_PER_LINE;
	offs = offs & ~(JFFS2_BUFDUMP_BYTES_PER_LINE - 1);
	
	if (skip != 0)
		printk(KERN_DEBUG "%#08x: ", offs);
	
	while (skip--)
		printk("   ");

	while (i < len) {
		if ((i % JFFS2_BUFDUMP_BYTES_PER_LINE) == 0 && i != len -1) {
			if (i != 0)
				printk("\n");
			offs += JFFS2_BUFDUMP_BYTES_PER_LINE;
			printk(KERN_DEBUG "%0#8x: ", offs);
		}

		printk("%02x ", buf[i]);
		
		i += 1;
	}

	printk("\n");
}
#endif /* JFFS2_DBG_DUMPS || JFFS2_DBG_PARANOIA_CHECKS */

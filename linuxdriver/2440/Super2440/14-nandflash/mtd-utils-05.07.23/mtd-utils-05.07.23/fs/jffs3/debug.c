/*
 * JFFS3 -- Journalling Flash File System, Version 3.
 *
 * Copyright (C) 2001-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@infradead.org>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * $Id: debug.c,v 1.11 2005/01/05 16:18:59 dedekind Exp $
 */
#include <linux/kernel.h>
#include <linux/pagemap.h>
#include <linux/mtd/mtd.h>
#include "jffs3.h"
#include "nodelist.h"
#include "debug.h"

#if PARANOIA > 0

#define JFFS3_DBG_BUF_SIZE	4096
static unsigned char dbg_buf[JFFS3_DBG_BUF_SIZE];

#define JFFS3_BUFDUMP_BYTES_PER_LINE	8
void
jffs3_dbg_dump_buffer(char *buf, int len, uint32_t offs)
{
	int i = 0;
	int skip = offs & ~(JFFS3_BUFDUMP_BYTES_PER_LINE - 1);

	while (i < len) {
		int j = 0;

		printk(KERN_DEBUG "0x#x: \n");
		while (skip) {
			printk("   ");
			skip -= 1;
		}

		while (j < JFFS3_BUFDUMP_BYTES_PER_LINE) {
			if (likely(i + j) < len)
				printk(" %#02x", buf[i + j++]);
		}

		i += JFFS3_BUFDUMP_BYTES_PER_LINE;
	}
}

/*
 * Check the space accounting and node_ref list correctness for the JFFS3 erasable block 'jeb'.
 */
void
jffs3_dbg_acct_paranoia_check(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb)
{
	uint32_t my_used_size = 0;
	uint32_t my_unchecked_size = 0;
	uint32_t my_dirty_size = 0;
	struct jffs3_raw_node_ref *ref2 = jeb->first_node;

	while (ref2) {
		uint32_t totlen = ref_totlen(c, jeb, ref2);

		if (unlikely(ref2->flash_offset < jeb->offset
				|| ref2->flash_offset > jeb->offset + c->sector_size)) {
			ERROR_MSG("node_ref %#08x shouldn't be in block at %#08x!\n",
				ref_offset(ref2), jeb->offset);
			jffs3_dbg_dump_node_refs(c, jeb);
			jffs3_dbg_dump_block_lists(c);
			BUG();

		}
		if (ref_flags(ref2) == REF_UNCHECKED)
			my_unchecked_size += totlen;
		else if (!ref_obsolete(ref2))
			my_used_size += totlen;
		else
			my_dirty_size += totlen;

		if (unlikely((!ref2->next_phys) != (ref2 == jeb->last_node))) {
			ERROR_MSG("node_ref for node at %#08x (mem %p) has next_phys at %#08x (mem %p), "
					"last_node is at %#08x (mem %p)\n",
					ref_offset(ref2), ref2, ref_offset(ref2->next_phys), ref2->next_phys,
					ref_offset(jeb->last_node), jeb->last_node);
			jffs3_dbg_dump_node_refs(c, jeb);
			jffs3_dbg_dump_block_lists(c);
			BUG();
		}
		ref2 = ref2->next_phys;
	}

	if (my_used_size != jeb->used_size) {
		ERROR_MSG("Calculated used size %#08x != stored used size %#08x\n",
				my_used_size, jeb->used_size);
		jffs3_dbg_dump_node_refs(c, jeb);
		jffs3_dbg_dump_block_lists(c);
		BUG();
	}

	if (my_unchecked_size != jeb->unchecked_size) {
		ERROR_MSG("Calculated unchecked size %#08x != stored unchecked size %#08x\n",
				my_unchecked_size, jeb->unchecked_size);
		jffs3_dbg_dump_node_refs(c, jeb);
		jffs3_dbg_dump_block_lists(c);
		BUG();
	}

	if (my_dirty_size != jeb->dirty_size + jeb->wasted_size) {
		ERROR_MSG("Calculated dirty+wasted size %#08x != stored dirty + wasted size %#08x\n",
			my_dirty_size, jeb->dirty_size + jeb->wasted_size);
		jffs3_dbg_dump_node_refs(c, jeb);
		jffs3_dbg_dump_block_lists(c);
		BUG();
	}

	if (jeb->free_size == 0
		&& my_used_size + my_unchecked_size + my_dirty_size != c->sector_size) {
		ERROR_MSG("The sum of all nodes in block (%#x) != size of block (%#x)\n",
			my_used_size + my_unchecked_size + my_dirty_size,
			c->sector_size);
		jffs3_dbg_dump_node_refs(c, jeb);
		jffs3_dbg_dump_block_lists(c);
		BUG();
	}
}

/*
 * Check if the flash contains all 0xFF before write.
 */
void
jffs3_dbg_prewrite_paranoia_check(struct jffs3_sb_info *c, uint32_t ofs, int len)
{
	size_t retlen;
	int ret, i;
	int len1 = 0;

	while (len1 < len) {
		len1 = len > JFFS3_DBG_BUF_SIZE ? JFFS3_DBG_BUF_SIZE : len;
		ret = jffs3_flash_read(c, ofs, len1, &retlen, &dbg_buf[0]);
		if (ret || (retlen != len1)) {
			WARNING_MSG("read %d bytes failed or short in %s(). ret %d, retlen %zd\n",
					len1, __FUNCTION__, ret, retlen);
			return;
		}

		ret = 0;
		for (i = 0; i < len1; i++) {
			if (dbg_buf[i] != 0xff) {
				ret = 1;
			}
		}

		if (ret) {
			ERROR_MSG("ARGH. About to write node to %#08x on flash, but there are data "
					"already there. The first corrupted byte is at %#08x.\n", ofs, ofs + i);
			jffs3_dbg_dump_buffer(dbg_buf, len1, ofs);
			BUG();
		}

		ofs += len1;
		len1 += len;
	}
}


void
jffs3_dbg_fragtree_paranoia_check(struct jffs3_inode_info *f)
{
	struct jffs3_node_frag *frag;
	int bitched = 0;

	for (frag = frag_first(&f->fragtree); frag; frag = frag_next(frag)) {
		struct jffs3_full_dnode *fn = frag->node;

		if (!fn || !fn->raw)
			continue;

		if (ref_flags(fn->raw) == REF_PRISTINE) {
			if (fn->frags > 1) {
				ERROR_MSG("REF_PRISTINE node at 0x%08x had %d frags. Tell dwmw2\n",
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
				ERROR_MSG("REF_PRISTINE node at 0x%08x had a previous non-hole frag "
						"in the same page. Tell dwmw2\n", ref_offset(fn->raw));
				bitched = 1;
			}

			if ((frag->ofs+frag->size) & (PAGE_CACHE_SIZE-1) && frag_next(frag)
					&& frag_next(frag)->size < PAGE_CACHE_SIZE && frag_next(frag)->node) {
				ERROR_MSG("REF_PRISTINE node at 0x%08x (%08x-%08x) had a following "
						"non-hole frag in the same page. Tell dwmw2\n",
					       ref_offset(fn->raw), frag->ofs, frag->ofs+frag->size);
				bitched = 1;
			}
		}
	}

	if (bitched) {
		ERROR_MSG("Fragtree is corrupted. Fragtree dump:\n");
		jffs3_dbg_dump_frag_list(f);
		BUG();
	}
}

#endif /* PARANOIA > 0 */

#if DEBUG1 > 0 || PARANOIA > 0

void
jffs3_dbg_dump_block_lists(struct jffs3_sb_info *c)
{
	printk(KERN_DEBUG "flash_size: %#08x\n",	c->flash_size);
	printk(KERN_DEBUG "used_size: %#08x\n",		c->used_size);
	printk(KERN_DEBUG "dirty_size: %#08x\n",	c->dirty_size);
	printk(KERN_DEBUG "wasted_size: %#08x\n",	c->wasted_size);
	printk(KERN_DEBUG "unchecked_size: %#08x\n",	c->unchecked_size);
	printk(KERN_DEBUG "free_size: %#08x\n",		c->free_size);
	printk(KERN_DEBUG "erasing_size: %#08x\n",	c->erasing_size);
	printk(KERN_DEBUG "bad_size: %#08x\n",		c->bad_size);
	printk(KERN_DEBUG "sector_size: %#08x\n",	c->sector_size);
	printk(KERN_DEBUG "jffs3_reserved_blocks size: %#08x\n",
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
			struct jffs3_eraseblock *jeb = list_entry(this, struct jffs3_eraseblock, list);
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
			struct jffs3_eraseblock *jeb = list_entry(this, struct jffs3_eraseblock, list);

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
			struct jffs3_eraseblock *jeb = list_entry(this, struct jffs3_eraseblock, list);

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
			struct jffs3_eraseblock *jeb = list_entry(this, struct jffs3_eraseblock, list);

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
			struct jffs3_eraseblock *jeb = list_entry(this, struct jffs3_eraseblock, list);

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
			struct jffs3_eraseblock *jeb = list_entry(this, struct jffs3_eraseblock, list);

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
			struct jffs3_eraseblock *jeb = list_entry(this, struct jffs3_eraseblock, list);

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
			struct jffs3_eraseblock *jeb = list_entry(this, struct jffs3_eraseblock, list);

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
			struct jffs3_eraseblock *jeb = list_entry(this, struct jffs3_eraseblock, list);

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
			struct jffs3_eraseblock *jeb = list_entry(this, struct jffs3_eraseblock, list);

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
jffs3_dbg_dump_frag_list(struct jffs3_inode_info *f)
{
	struct jffs3_node_frag *this = frag_first(&f->fragtree);
	uint32_t lastofs = 0;
	int buggy = 0;

	printk(KERN_DEBUG "inode is ino #%u\n", f->inocache->ino);
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
		lastofs = this->ofs+this->size;
		this = frag_next(this);
	}

	if (f->metadata)
		printk(KERN_DEBUG "metadata at 0x%08x\n", ref_offset(f->metadata->raw));

	if (buggy) {
		ERROR_MSG("Error! %s(): Frag tree got a hole in it\n", __FUNCTION__);
		BUG();
	}
}

void
jffs3_dbg_dump_dirents_list(struct jffs3_full_dirent *list)
{
	int i = 0;

	while(list) {
		printk(KERN_DEBUG "%d. Dirent \"%s\" (hash 0x%08x, ino #%u)\n",
				i++, list->name, list->nhash, list->ino);
		list = list->next;
	}
}

/*
 * Dump the node_refs of the 'jeb' JFFS3 eraseblock.
 */
void
jffs3_dbg_dump_node_refs(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb) {
	struct jffs3_raw_node_ref *ref;
	int i = 0;

	if (!jeb->first_node) {
		printk(KERN_DEBUG "no nodes in block %#08x\n", jeb->offset);
		return;
	}

	printk(KERN_DEBUG);
	for (ref = jeb->first_node; ; ref = ref->next_phys) {
#ifdef TMP_TOTLEN
		printk("%#08x(%#x)", ref_offset(ref), ref->__totlen);
#else
		printk("%#08x(%#x)", ref_offset(ref), ref_totlen(c, jeb, ref));
#endif
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

#endif /* DEBUG1 > 0 || PARANOIA > 0 */

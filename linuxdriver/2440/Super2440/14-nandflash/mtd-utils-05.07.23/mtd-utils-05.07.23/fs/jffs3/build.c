/*
 * JFFS3 -- Journalling Flash File System, Version 3.
 *
 * Copyright (C) 2001-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@infradead.org>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * JFFS2 Id: build.c,v 1.68 2004/11/27 13:38:10 gleixner  Exp
 * $Id: build.c,v 3.8 2005/01/05 16:18:59 dedekind Exp $
 *
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mtd/mtd.h>
#include "nodelist.h"

static void jffs3_build_remove_unlinked_inode(struct jffs3_sb_info *, struct jffs3_inode_cache *, struct jffs3_full_dirent **);

static inline struct jffs3_inode_cache *
first_inode_chain(int *i, struct jffs3_sb_info *c)
{
	for (; *i < INOCACHE_HASHSIZE; (*i)++) {
		if (c->inocache_list[*i])
			return c->inocache_list[*i];
	}
	return NULL;
}

static inline struct jffs3_inode_cache *
next_inode(int *i, struct jffs3_inode_cache *ic, struct jffs3_sb_info *c)
{
	/* More in this chain? */
	if (ic->next)
		return ic->next;
	(*i)++;
	return first_inode_chain(i, c);
}

#define for_each_inode(i, c, ic)			\
	for (i = 0, ic = first_inode_chain(&i, (c));	\
	     ic;					\
	     ic = next_inode(&i, ic, (c)))


static inline void jffs3_build_inode_pass1(struct jffs3_sb_info *c, struct jffs3_inode_cache *ic)
{
	struct jffs3_full_dirent *fd;

	DBG_BLD(1, "building directory inode #%u\n", ic->ino);

	/* For each child, increase nlink */
	for(fd = ic->scan_dents; fd; fd = fd->next) {
		struct jffs3_inode_cache *child_ic;
		if (!fd->ino)
			continue;

		/* XXX: Can get high latency here with huge directories */

		child_ic = jffs3_get_ino_cache(c, fd->ino);
		if (!child_ic) {
			NOTICE_MSG("Eep. Child \"%s\" (ino #%u) of dir ino #%u doesn't exist!\n",
					fd->name, fd->ino, ic->ino);
			jffs3_mark_node_obsolete(c, fd->raw);
			continue;
		}

		if (child_ic->nlink++ && fd->type == DT_DIR) {
			NOTICE_MSG("Child dir \"%s\" (ino #%u) of dir ino #%u appears to be a hard link\n",
					fd->name, fd->ino, ic->ino);
			if (fd->ino == 1 && ic->ino == 1) {
				NOTICE_MSG("This is mostly harmless, and probably caused by creating a JFFS3 image\n");
				NOTICE_MSG("using a buggy version of mkfs.jffs3. Use at least v1.17.\n");
			}
			/* What do we do about it? */
		}
		DBG_BLD(1, "Increased nlink for child \"%s\" (ino #%u)\n", fd->name, fd->ino);
		/* Can't free them. We might need them in pass 2 */
	}
}

/* Scan plan:
 - Scan physical nodes. Build map of inodes/dirents. Allocate inocaches as we go
 - Scan directory tree from top down, setting nlink in inocaches
 - Scan inocaches for inodes with nlink==0
*/
static int jffs3_build_filesystem(struct jffs3_sb_info *c)
{
	int ret;
	int i;
	struct jffs3_inode_cache *ic;
	struct jffs3_full_dirent *fd;
	struct jffs3_full_dirent *dead_fds = NULL;

	/* First, scan the medium and build all the inode caches with
	   lists of physical nodes */

	c->flags |= JFFS3_SB_FLAG_MOUNTING;
	ret = jffs3_scan_medium(c);
	if (ret)
		goto exit;

	DBG_BLD(1, "Scanned flash completely\n");
	if (DEBUG2) {
		DBG_BLD(2, "Blocks info:\n");
		jffs3_dbg_dump_block_lists(c);
	}

	/* Now scan the directory tree, increasing nlink according to every dirent found. */
	for_each_inode(i, c, ic) {
		DBG_BLD(1, "Pass 1: ino #%u\n", ic->ino);
		if (SANITY)
			BUG_ON(ic->ino > c->highest_ino);
		if (ic->scan_dents) {
			jffs3_build_inode_pass1(c, ic);
			cond_resched();
		}
	}
	c->flags &= ~JFFS3_SB_FLAG_MOUNTING;

	DBG_BLD(1, "Pass 1 complete\n");

	/* Next, scan for inodes with nlink == 0 and remove them. If
	   they were directories, then decrement the nlink of their
	   children too, and repeat the scan. As that's going to be
	   a fairly uncommon occurrence, it's not so evil to do it this
	   way. Recursion bad. */
	DBG_BLD(1, "Pass 2 starting\n");

	for_each_inode(i, c, ic) {
		DBG_BLD(1, "Pass 2: ino #%u, nlink %d, ic %p, nodes %p\n",
				ic->ino, ic->nlink, ic, ic->nodes);
		if (ic->nlink)
			continue;

		jffs3_build_remove_unlinked_inode(c, ic, &dead_fds);
		cond_resched();
	}

	DBG_BLD(1, "Pass 2a starting\n");

	while (dead_fds) {
		fd = dead_fds;
		dead_fds = fd->next;

		ic = jffs3_get_ino_cache(c, fd->ino);
		DBG_BLD(1, "Removing dead_fd ino #%u (\"%s\"), ic at %p\n", fd->ino, fd->name, ic);

		if (ic)
			jffs3_build_remove_unlinked_inode(c, ic, &dead_fds);
		jffs3_free_full_dirent(fd);
	}

	DBG_BLD(1, "Pass 2 complete\n");

	/* Finally, we can scan again and free the dirent structs */
	for_each_inode(i, c, ic) {
		DBG_BLD(1, "Pass 3: ino #%u, ic %p, nodes %p\n", ic->ino, ic, ic->nodes);

		while(ic->scan_dents) {
			fd = ic->scan_dents;
			ic->scan_dents = fd->next;
			jffs3_free_full_dirent(fd);
		}
		ic->scan_dents = NULL;
		cond_resched();
	}
	DBG_BLD(1, "Pass 3 complete\n");
	if (DEBUG2) {
		DBG_BLD(2, "Blocks info:\n");
		jffs3_dbg_dump_block_lists(c);
	}

	/* Rotate the lists by some number to ensure wear levelling */
	jffs3_rotate_lists(c);

	ret = 0;

exit:
	if (ret) {
		for_each_inode(i, c, ic) {
			while(ic->scan_dents) {
				fd = ic->scan_dents;
				ic->scan_dents = fd->next;
				jffs3_free_full_dirent(fd);
			}
		}
	}

	return ret;
}

static void jffs3_build_remove_unlinked_inode(struct jffs3_sb_info *c, struct jffs3_inode_cache *ic, struct jffs3_full_dirent **dead_fds)
{
	struct jffs3_raw_node_ref *raw;
	struct jffs3_full_dirent *fd;

	DBG_BLD(1, "Removing ino #%u with nlink == zero.\n", ic->ino);

	raw = ic->nodes;
	while (raw != (void *)ic) {
		struct jffs3_raw_node_ref *next = raw->next_in_ino;
		DBG_BLD(1, "obsoleting node at 0x%08x\n", ref_offset(raw));
		jffs3_mark_node_obsolete(c, raw);
		raw = next;
	}

	if (ic->scan_dents) {
		int whinged = 0;
		DBG_BLD(1, "Inode #%u was a directory which may have children...\n", ic->ino);

		while(ic->scan_dents) {
			struct jffs3_inode_cache *child_ic;

			fd = ic->scan_dents;
			ic->scan_dents = fd->next;

			if (!fd->ino) {
				/* It's a deletion dirent. Ignore it */
				DBG_BLD(1, "Child \"%s\" is a deletion dirent, skipping...\n", fd->name);
				jffs3_free_full_dirent(fd);
				continue;
			}
			if (!whinged) {
				whinged = 1;
				NOTICE_MSG("Inode #%u was a directory with children - "
						"removing those too...\n", ic->ino);
			}

			DBG_BLD(1, "Removing child \"%s\", ino #%u\n", fd->name, fd->ino);

			child_ic = jffs3_get_ino_cache(c, fd->ino);
			if (!child_ic) {
				NOTICE_MSG("Cannot remove child \"%s\", ino #%u, "
					"because it doesn't exist\n", fd->name, fd->ino);
				jffs3_free_full_dirent(fd);
				continue;
			}

			/* Reduce nlink of the child. If it's now zero, stick it on the
			   dead_fds list to be cleaned up later. Else just free the fd */

			child_ic->nlink--;

			if (!child_ic->nlink) {
				DBG_BLD(1, "Inode #%u (\"%s\") has now got zero nlink. Adding to dead_fds list.\n",
					  fd->ino, fd->name);
				fd->next = *dead_fds;
				*dead_fds = fd;
			} else {
				DBG_BLD(1, "Inode #%u (\"%s\") has now got nlink %d. Ignoring.\n",
					  fd->ino, fd->name, child_ic->nlink);
				jffs3_free_full_dirent(fd);
			}
		}
	}

	/*
	   We don't delete the inocache from the hash list and free it yet.
	   The erase code will do that, when all the nodes are completely gone.
	*/
}

static void jffs3_calc_trigger_levels(struct jffs3_sb_info *c)
{
	uint32_t size;

	/* Deletion should almost _always_ be allowed. We're fairly
	   buggered once we stop allowing people to delete stuff
	   because there's not enough free space... */
	c->resv_blocks_deletion = 2;

	/* Be conservative about how much space we need before we allow writes.
	   On top of that which is required for deletia, require an extra 2%
	   of the medium to be available, for overhead caused by nodes being
	   split across blocks, etc. */

	size = c->flash_size / 50; /* 2% of flash size */
	size += c->nr_blocks * 100; /* And 100 bytes per eraseblock */
	size += c->sector_size - 1; /* ... and round up */

	c->resv_blocks_write = c->resv_blocks_deletion + (size / c->sector_size);

	/* When do we let the GC thread run in the background */

	c->resv_blocks_gctrigger = c->resv_blocks_write + 1;

	/* When do we allow garbage collection to merge nodes to make
	   long-term progress at the expense of short-term space exhaustion? */
	c->resv_blocks_gcmerge = c->resv_blocks_deletion + 1;

	/* When do we allow garbage collection to eat from bad blocks rather
	   than actually making progress? */
	c->resv_blocks_gcbad = 0;//c->resv_blocks_deletion + 2;

	/* If there's less than this amount of dirty space, don't bother
	   trying to GC to make more space. It'll be a fruitless task */
	c->nospc_dirty_size = c->sector_size + (c->flash_size / 100);

	DBG_BLD(1, "JFFS3 trigger levels (size %d KiB, block size %d KiB, %d blocks)\n",
		  c->flash_size / 1024, c->sector_size / 1024, c->nr_blocks);
	DBG_BLD(1, "Blocks required to allow deletion:    %d (%d KiB)\n",
		  c->resv_blocks_deletion, c->resv_blocks_deletion*c->sector_size/1024);
	DBG_BLD(1, "Blocks required to allow writes:      %d (%d KiB)\n",
		  c->resv_blocks_write, c->resv_blocks_write*c->sector_size/1024);
	DBG_BLD(1, "Blocks required to quiesce GC thread: %d (%d KiB)\n",
		  c->resv_blocks_gctrigger, c->resv_blocks_gctrigger*c->sector_size/1024);
	DBG_BLD(1, "Blocks required to allow GC merges:   %d (%d KiB)\n",
		  c->resv_blocks_gcmerge, c->resv_blocks_gcmerge*c->sector_size/1024);
	DBG_BLD(1, "Blocks required to GC bad blocks:     %d (%d KiB)\n",
		  c->resv_blocks_gcbad, c->resv_blocks_gcbad*c->sector_size/1024);
	DBG_BLD(1, "Amount of dirty space required to GC: %d bytes\n",
		  c->nospc_dirty_size);
}

int jffs3_do_mount_fs(struct jffs3_sb_info *c)
{
	int i;

	c->free_size = c->flash_size;
	c->nr_blocks = c->flash_size / c->sector_size;
 	if (c->mtd->flags & MTD_NO_VIRTBLOCKS)
		c->blocks = vmalloc(sizeof(struct jffs3_eraseblock) * c->nr_blocks);
	else
		c->blocks = kmalloc(sizeof(struct jffs3_eraseblock) * c->nr_blocks, GFP_KERNEL);
	if (!c->blocks)
		return -ENOMEM;
	for (i=0; i<c->nr_blocks; i++) {
		INIT_LIST_HEAD(&c->blocks[i].list);
		c->blocks[i].offset = i * c->sector_size;
		c->blocks[i].free_size = c->sector_size;
		c->blocks[i].dirty_size = 0;
		c->blocks[i].wasted_size = 0;
		c->blocks[i].unchecked_size = 0;
		c->blocks[i].used_size = 0;
		c->blocks[i].first_node = NULL;
		c->blocks[i].last_node = NULL;
		c->blocks[i].bad_count = 0;
#ifdef CONFIG_JFFS3_SUMMARY
 		c->blocks[i].sum_collected = NULL;
#endif
	}

	init_MUTEX(&c->alloc_sem);
	init_MUTEX(&c->erase_free_sem);
	init_waitqueue_head(&c->erase_wait);
	init_waitqueue_head(&c->inocache_wq);
	spin_lock_init(&c->erase_completion_lock);
	spin_lock_init(&c->inocache_lock);

	INIT_LIST_HEAD(&c->clean_list);
	INIT_LIST_HEAD(&c->very_dirty_list);
	INIT_LIST_HEAD(&c->dirty_list);
	INIT_LIST_HEAD(&c->erasable_list);
	INIT_LIST_HEAD(&c->erasing_list);
	INIT_LIST_HEAD(&c->erase_pending_list);
	INIT_LIST_HEAD(&c->erasable_pending_wbuf_list);
	INIT_LIST_HEAD(&c->erase_complete_list);
	INIT_LIST_HEAD(&c->free_list);
	INIT_LIST_HEAD(&c->bad_list);
	INIT_LIST_HEAD(&c->bad_used_list);
	c->highest_ino = 1;

	if (jffs3_build_filesystem(c)) {
		DBG_BLD(1, "build_fs failed\n");
		jffs3_free_ino_caches(c);
		jffs3_free_raw_node_refs(c);
		kfree(c->blocks);
		return -EIO;
	}

	jffs3_calc_trigger_levels(c);

	return 0;
}

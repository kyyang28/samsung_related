/*
 * JFFS3 -- Journalling Flash File System, Version 3.
 *
 * Copyright (C) 2001-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@infradead.org>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * JFFS2 Id: file.c,v 1.99 2004/11/16 20:36:11 dwmw2  Exp
 * $Id: file.c,v 3.7 2005/01/05 16:19:00 dedekind Exp $
 *
 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/pagemap.h>
#include <linux/highmem.h>
#include <linux/crc32.h>
#include "jffs3.h"
#include "nodelist.h"
#include "summary.h"

extern int generic_file_open(struct inode *, struct file *) __attribute__((weak));
extern loff_t generic_file_llseek(struct file *file, loff_t offset, int origin) __attribute__((weak));


int jffs3_fsync(struct file *filp, struct dentry *dentry, int datasync)
{
	struct inode *inode = dentry->d_inode;
	struct jffs3_sb_info *c = JFFS3_SB_INFO(inode->i_sb);

	/* Trigger GC to flush any pending writes for this inode */
	jffs3_flush_wbuf_gc(c, inode->i_ino);

	return 0;
}

struct file_operations jffs3_file_operations =
{
	.llseek =	generic_file_llseek,
	.open =		generic_file_open,
	.read =		generic_file_read,
	.write =	generic_file_write,
	.ioctl =	jffs3_ioctl,
	.mmap =		generic_file_readonly_mmap,
	.fsync =	jffs3_fsync,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,29)
	.sendfile =	generic_file_sendfile
#endif
};

/* jffs3_file_inode_operations */

struct inode_operations jffs3_file_inode_operations =
{
	.setattr =	jffs3_setattr
};

struct address_space_operations jffs3_file_address_operations =
{
	.readpage =	jffs3_readpage,
	.prepare_write =jffs3_prepare_write,
	.commit_write =	jffs3_commit_write
};

int jffs3_do_readpage_nolock (struct inode *inode, struct page *pg)
{
	struct jffs3_inode_info *f = JFFS3_INODE_INFO(inode);
	struct jffs3_sb_info *c = JFFS3_SB_INFO(inode->i_sb);
	unsigned char *pg_buf;
	int ret;

	DBG_VFS(2, "Inode #%lu, page at offset 0x%lx\n",
		inode->i_ino, pg->index << PAGE_CACHE_SHIFT);

	if (!PageLocked(pg))
                PAGE_BUG(pg);

	pg_buf = kmap(pg);
	/* FIXME: Can kmap fail? */

	ret = jffs3_read_inode_range(c, f, pg_buf, pg->index << PAGE_CACHE_SHIFT, PAGE_CACHE_SIZE);

	if (ret) {
		ClearPageUptodate(pg);
		SetPageError(pg);
	} else {
		SetPageUptodate(pg);
		ClearPageError(pg);
	}

	flush_dcache_page(pg);
	kunmap(pg);

	DBG_VFS(2,"Finished\n");
	return 0;
}

int jffs3_do_readpage_unlock(struct inode *inode, struct page *pg)
{
	int ret = jffs3_do_readpage_nolock(inode, pg);
	unlock_page(pg);
	return ret;
}


int jffs3_readpage (struct file *filp, struct page *pg)
{
	struct jffs3_inode_info *f = JFFS3_INODE_INFO(pg->mapping->host);
	int ret;

	down(&f->sem);
	ret = jffs3_do_readpage_unlock(pg->mapping->host, pg);
	up(&f->sem);
	return ret;
}

int jffs3_prepare_write (struct file *filp, struct page *pg, unsigned start, unsigned end)
{
	struct inode *inode = pg->mapping->host;
	struct jffs3_inode_info *f = JFFS3_INODE_INFO(inode);
	uint32_t pageofs = pg->index << PAGE_CACHE_SHIFT;
	int ret = 0;

	DBG_VFS(1, "Prepare write %#x-%#x\n", start, end);

	if (pageofs > inode->i_size) {
		/* Make new hole frag from old EOF to new page */
		struct jffs3_sb_info *c = JFFS3_SB_INFO(inode->i_sb);
		struct jffs3_raw_inode ri;
		struct jffs3_full_dnode *fn;
		uint32_t phys_ofs, alloc_len;

		DBG_VFS(1, "Writing new hole frag %#x-%#x between current EOF and new page\n",
			  (unsigned int)inode->i_size, pageofs);

		ret = jffs3_reserve_space(c, sizeof(ri), &phys_ofs, &alloc_len, ALLOC_NORMAL, JFFS3_SUMMARY_INODE_SIZE);
		if (ret)
			return ret;

		down(&f->sem);
		memset(&ri, 0, sizeof(ri));

		ri.magic = cpu_to_je16(JFFS3_MAGIC_BITMASK);
		ri.nodetype = cpu_to_je16(JFFS3_NODETYPE_INODE);
		ri.totlen = cpu_to_je32(sizeof(ri));
		ri.hdr_crc = cpu_to_je32(crc32(0, &ri, sizeof(struct jffs3_unknown_node)-4));

		ri.ino = cpu_to_je32(f->inocache->ino);
		ri.version = cpu_to_je32(++f->highest_version);
		ri.mode = cpu_to_jemode(inode->i_mode);
		ri.uid = cpu_to_je16(inode->i_uid);
		ri.gid = cpu_to_je16(inode->i_gid);
		ri.isize = cpu_to_je32(max((uint32_t)inode->i_size, pageofs));
		ri.atime = ri.ctime = ri.mtime = cpu_to_je32(get_seconds());
		ri.offset = cpu_to_je32(inode->i_size);
		ri.dsize = cpu_to_je32(pageofs - inode->i_size);
		ri.csize = cpu_to_je32(0);
		ri.compr = JFFS3_COMPR_ZERO;
		ri.node_crc = cpu_to_je32(crc32(0, &ri, sizeof(ri)-8));
		ri.data_crc = cpu_to_je32(0);

		fn = jffs3_write_dnode(c, f, &ri, NULL, 0, phys_ofs, ALLOC_NORMAL);

		if (IS_ERR(fn)) {
			ret = PTR_ERR(fn);
			jffs3_complete_reservation(c);
			up(&f->sem);
			return ret;
		}
		ret = jffs3_add_full_dnode_to_inode(c, f, fn);
		if (f->metadata) {
			jffs3_mark_node_obsolete(c, f->metadata->raw);
			jffs3_free_full_dnode(f->metadata);
			f->metadata = NULL;
		}
		if (ret) {
			DBG_VFS(1, "Eep. add_full_dnode_to_inode() failed in prepare_write, returned %d\n", ret);
			jffs3_mark_node_obsolete(c, fn->raw);
			jffs3_free_full_dnode(fn);
			jffs3_complete_reservation(c);
			up(&f->sem);
			return ret;
		}
		jffs3_complete_reservation(c);
		inode->i_size = pageofs;
		up(&f->sem);
	}

	/* Read in the page if it wasn't already present, unless it's a whole page */
	if (!PageUptodate(pg) && (start || end < PAGE_CACHE_SIZE)) {
		down(&f->sem);
		ret = jffs3_do_readpage_nolock(inode, pg);
		up(&f->sem);
	}
	DBG_VFS(1, "Finished. pg->flags %lx\n", pg->flags);
	return ret;
}

int jffs3_commit_write (struct file *filp, struct page *pg, unsigned start, unsigned end)
{
	/* Actually commit the write from the page cache page we're looking at.
	 * For now, we write the full page out each time. It sucks, but it's simple
	 */
	struct inode *inode = pg->mapping->host;
	struct jffs3_inode_info *f = JFFS3_INODE_INFO(inode);
	struct jffs3_sb_info *c = JFFS3_SB_INFO(inode->i_sb);
	struct jffs3_raw_inode *ri;
	unsigned aligned_start = start & ~3;
	int ret = 0;
	uint32_t writtenlen = 0;

	DBG_VFS(1, "Inode #%lu, page %#lx, range %#x-%#x, flags %#lx\n",
		  inode->i_ino, pg->index << PAGE_CACHE_SHIFT, start, end, pg->flags);

	if (!start && end == PAGE_CACHE_SIZE) {
		/* We need to avoid deadlock with page_cache_read() in
		   jffs3_garbage_collect_pass(). So we have to mark the
		   page up to date, to prevent page_cache_read() from
		   trying to re-lock it. */
		SetPageUptodate(pg);
	}

	ri = jffs3_alloc_raw_inode();

	if (!ri) {
		WARNING_MSG("Allocation of raw inode failed\n");
		return -ENOMEM;
	}

	/* Set the fields that the generic jffs3_write_inode_range() code can't find */
	ri->ino = cpu_to_je32(inode->i_ino);
	ri->mode = cpu_to_jemode(inode->i_mode);
	ri->uid = cpu_to_je16(inode->i_uid);
	ri->gid = cpu_to_je16(inode->i_gid);
	ri->isize = cpu_to_je32((uint32_t)inode->i_size);
	ri->atime = ri->ctime = ri->mtime = cpu_to_je32(get_seconds());

	/* In 2.4, it was already kmapped by generic_file_write(). Doesn't
	   hurt to do it again. The alternative is ifdefs, which are ugly. */
	kmap(pg);

	ret = jffs3_write_inode_range(c, f, ri, page_address(pg) + aligned_start,
				      (pg->index << PAGE_CACHE_SHIFT) + aligned_start,
				      end - aligned_start, &writtenlen);

	kunmap(pg);

	if (ret) {
		/* There was an error writing. */
		SetPageError(pg);
	}

	/* Adjust writtenlen for the padding we did, so we don't confuse our caller */
	if (writtenlen < (start&3))
		writtenlen = 0;
	else
		writtenlen -= (start&3);

	if (writtenlen) {
		if (inode->i_size < (pg->index << PAGE_CACHE_SHIFT) + start + writtenlen) {
			inode->i_size = (pg->index << PAGE_CACHE_SHIFT) + start + writtenlen;
			inode->i_blocks = (inode->i_size + 511) >> 9;

			inode->i_ctime = inode->i_mtime = ITIME(je32_to_cpu(ri->ctime));
		}
	}

	jffs3_free_raw_inode(ri);

	if (start+writtenlen < end) {
		/* generic_file_write has written more to the page cache than we've
		   actually written to the medium. Mark the page !Uptodate so that
		   it gets reread */
		DBG_VFS(1, "Not all bytes written. Marking page !uptodate\n");
		SetPageError(pg);
		ClearPageUptodate(pg);
	}

	DBG_VFS(1, "returning %d\n",writtenlen?writtenlen:ret);
	return writtenlen?writtenlen:ret;
}

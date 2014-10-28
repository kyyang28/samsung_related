/*
 * JFFS3 -- Journalling Flash File System, Version 3.
 *
 * Copyright (C) 2001-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@infradead.org>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * JFFS2 Id: super.c,v 1.104 2004/11/23 15:37:31 gleixner  Exp
 * $Id: super.c,v 3.10 2005/02/09 14:25:15 pavlov Exp $
 *
 */

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/pagemap.h>
#include <linux/mtd/mtd.h>
#include <linux/ctype.h>
#include <linux/namei.h>
#include "jffs3.h"
#include "compr.h"
#include "nodelist.h"
#include "summary.h"

static void jffs3_put_super(struct super_block *);

static kmem_cache_t *jffs3_inode_cachep;
struct inode *jffs3_alloc_inode(struct super_block *sb)
{
	struct jffs3_inode_info *ei;
	ei = (struct jffs3_inode_info *)kmem_cache_alloc(jffs3_inode_cachep, SLAB_KERNEL);
	DBG_SALLOC(1, "Allocated jffs3_inode_info at %p\n", ei);
	if (!ei)
		return NULL;
	return &ei->vfs_inode;
}

static void jffs3_destroy_inode(struct inode *inode)
{
	DBG_SALLOC(1, "Free jffs3_inode_info at %p\n", JFFS3_INODE_INFO(inode));
	kmem_cache_free(jffs3_inode_cachep, JFFS3_INODE_INFO(inode));
}

static void jffs3_i_init_once(void * foo, kmem_cache_t * cachep, unsigned long flags)
{
	struct jffs3_inode_info *ei = (struct jffs3_inode_info *) foo;

	if ((flags & (SLAB_CTOR_VERIFY|SLAB_CTOR_CONSTRUCTOR)) ==
	    SLAB_CTOR_CONSTRUCTOR) {
		init_MUTEX_LOCKED(&ei->sem);
		inode_init_once(&ei->vfs_inode);
	}
}

static int jffs3_sync_fs(struct super_block *sb, int wait)
{
	struct jffs3_sb_info *c = JFFS3_SB_INFO(sb);

	down(&c->alloc_sem);
	jffs3_flush_wbuf_pad(c);
	up(&c->alloc_sem);
	return 0;
}

static struct super_operations jffs3_super_operations =
{
	.alloc_inode =	jffs3_alloc_inode,
	.destroy_inode =jffs3_destroy_inode,
	.read_inode =	jffs3_read_inode,
	.put_super =	jffs3_put_super,
	.write_super =	jffs3_write_super,
	.statfs =	jffs3_statfs,
	.remount_fs =	jffs3_remount_fs,
	.clear_inode =	jffs3_clear_inode,
	.dirty_inode =	jffs3_dirty_inode,
	.sync_fs =	jffs3_sync_fs,
};

static int jffs3_sb_compare(struct super_block *sb, void *data)
{
	struct jffs3_sb_info *p = data;
	struct jffs3_sb_info *c = JFFS3_SB_INFO(sb);

	/* The superblocks are considered to be equivalent if the underlying MTD
	   device is the same one */
	if (c->mtd == p->mtd) {
		DBG_VFS(1, "match on device %d (\"%s\")\n", p->mtd->index, p->mtd->name);
		return 1;
	} else {
		DBG_VFS(1, "No match, device %d (\"%s\"), device %d (\"%s\")\n",
		  c->mtd->index, c->mtd->name, p->mtd->index, p->mtd->name);
		return 0;
	}
}

static int jffs3_sb_set(struct super_block *sb, void *data)
{
	struct jffs3_sb_info *p = data;

	/* For persistence of NFS exports etc. we use the same s_dev
	   each time we mount the device, don't just use an anonymous
	   device */
	sb->s_fs_info = p;
	p->os_priv = sb;
	sb->s_dev = MKDEV(MTD_BLOCK_MAJOR, p->mtd->index);

	return 0;
}

static struct super_block *jffs3_get_sb_mtd(struct file_system_type *fs_type,
					      int flags, const char *dev_name,
					      void *data, struct mtd_info *mtd)
{
	struct super_block *sb;
	struct jffs3_sb_info *c;
	int ret;

	c = kmalloc(sizeof(*c), GFP_KERNEL);
	if (!c)
		return ERR_PTR(-ENOMEM);
	memset(c, 0, sizeof(*c));
	c->mtd = mtd;

	sb = sget(fs_type, jffs3_sb_compare, jffs3_sb_set, c);

	if (IS_ERR(sb))
		goto out_put;

	if (sb->s_root) {
		/* New mountpoint for JFFS3 which is already mounted */
		DBG_VFS(1, "Device %d (\"%s\") is already mounted\n",
			  mtd->index, mtd->name);
		goto out_put;
	}

	DBG_VFS(1, "New superblock for device %d (\"%s\")\n",
		  mtd->index, mtd->name);

	sb->s_op = &jffs3_super_operations;
	sb->s_flags = flags | MS_NOATIME;

	ret = jffs3_do_fill_super(sb, data, (flags&MS_VERBOSE)?1:0);

	if (ret) {
		/* Failure case... */
		up_write(&sb->s_umount);
		deactivate_super(sb);
		return ERR_PTR(ret);
	}

#ifdef CONFIG_JFFS3_SUMMARY
        jffs3_sum_init(c);
#endif
	sb->s_flags |= MS_ACTIVE;
	return sb;

 out_put:
	kfree(c);
	put_mtd_device(mtd);

	return sb;
}

static struct super_block *jffs3_get_sb_mtdnr(struct file_system_type *fs_type,
					      int flags, const char *dev_name,
					      void *data, int mtdnr)
{
	struct mtd_info *mtd;

	mtd = get_mtd_device(NULL, mtdnr);
	if (!mtd) {
		ERROR_MSG("jffs3: MTD device #%u doesn't appear to exist\n", mtdnr);
		return ERR_PTR(-EINVAL);
	}

	return jffs3_get_sb_mtd(fs_type, flags, dev_name, data, mtd);
}

static struct super_block *jffs3_get_sb(struct file_system_type *fs_type,
					int flags, const char *dev_name,
					void *data)
{
	int err;
	struct nameidata nd;
	int mtdnr;

	if (!dev_name)
		return ERR_PTR(-EINVAL);

	DBG_VFS(1, "dev_name \"%s\"\n", dev_name);

	/* The preferred way of mounting in future; especially when
	   CONFIG_BLK_DEV is implemented - we specify the underlying
	   MTD device by number or by name, so that we don't require
	   block device support to be present in the kernel. */

	/* FIXME: How to do the root fs this way? */

	if (dev_name[0] == 'm' && dev_name[1] == 't' && dev_name[2] == 'd') {
		/* Probably mounting without the blkdev crap */
		if (dev_name[3] == ':') {
			struct mtd_info *mtd;

			/* Mount by MTD device name */
			DBG_VFS(1, "mtd:%%s, name \"%s\"\n", dev_name + 4);
			for (mtdnr = 0; mtdnr < MAX_MTD_DEVICES; mtdnr++) {
				mtd = get_mtd_device(NULL, mtdnr);
				if (mtd) {
					if (!strcmp(mtd->name, dev_name+4))
						return jffs3_get_sb_mtd(fs_type, flags, dev_name, data, mtd);
					put_mtd_device(mtd);
				}
			}
			NOTICE_MSG("MTD device with name \"%s\" not found.\n", dev_name+4);
		} else if (isdigit(dev_name[3])) {
			/* Mount by MTD device number name */
			char *endptr;

			mtdnr = simple_strtoul(dev_name+3, &endptr, 0);
			if (!*endptr) {
				/* It was a valid number */
				DBG_VFS(1, "mtd%%d, mtdnr %d\n", mtdnr);
				return jffs3_get_sb_mtdnr(fs_type, flags, dev_name, data, mtdnr);
			}
		}
	}

	/* Try the old way - the hack where we allowed users to mount
	   /dev/mtdblock$(n) but didn't actually _use_ the blkdev */

	err = path_lookup(dev_name, LOOKUP_FOLLOW, &nd);

	DBG_VFS(1, "path_lookup() returned %d, inode %p\n", err, nd.dentry->d_inode);

	if (err)
		return ERR_PTR(err);

	err = -EINVAL;

	if (!S_ISBLK(nd.dentry->d_inode->i_mode))
		goto out;

	if (nd.mnt->mnt_flags & MNT_NODEV) {
		err = -EACCES;
		goto out;
	}

	if (imajor(nd.dentry->d_inode) != MTD_BLOCK_MAJOR) {
		if (!(flags & MS_VERBOSE)) /* Yes I mean this. Strangely */
			NOTICE_MSG("Attempt to mount non-MTD device \"%s\" as JFFS3\n", dev_name);
		goto out;
	}

	mtdnr = iminor(nd.dentry->d_inode);
	path_release(&nd);

	return jffs3_get_sb_mtdnr(fs_type, flags, dev_name, data, mtdnr);

out:
	path_release(&nd);
	return ERR_PTR(err);
}

static void jffs3_put_super (struct super_block *sb)
{
	struct jffs3_sb_info *c = JFFS3_SB_INFO(sb);

	DBG_VFS(1, "enter\n");

	if (!(sb->s_flags & MS_RDONLY))
		jffs3_stop_garbage_collect_thread(c);
	down(&c->alloc_sem);
	jffs3_flush_wbuf_pad(c);
	up(&c->alloc_sem);
#ifdef CONFIG_JFFS3_SUMMARY
	jffs3_sum_clean_all_info(c);
	jffs3_sum_exit(c);
#endif
	jffs3_free_ino_caches(c);
	jffs3_free_raw_node_refs(c);
	if (c->mtd->flags & MTD_NO_VIRTBLOCKS)
		vfree(c->blocks);
	else
		kfree(c->blocks);
	jffs3_flash_cleanup(c);
	kfree(c->inocache_list);
	if (c->mtd->sync)
		c->mtd->sync(c->mtd);

	DBG_VFS(1, "returning\n");
}

static void jffs3_kill_sb(struct super_block *sb)
{
	struct jffs3_sb_info *c = JFFS3_SB_INFO(sb);
	generic_shutdown_super(sb);
	put_mtd_device(c->mtd);
	kfree(c);
}

static struct file_system_type jffs3_fs_type = {
	.owner =	THIS_MODULE,
	.name =		"jffs3",
	.get_sb =	jffs3_get_sb,
	.kill_sb =	jffs3_kill_sb,
};

static int __init init_jffs3_fs(void)
{
	int ret;

	NOTICE_MSG("JFFS3 version 2.9."
#ifdef CONFIG_JFFS3_FS_WRITEBUFFER
	       " (NAND)"
#endif
#ifdef CONFIG_JFFS3_SUMMARY
           " (SUMMARY) "
#endif
	       " (C) 2001-2004 Red Hat, Inc.\n");

	jffs3_inode_cachep = kmem_cache_create("jffs3_i",
					     sizeof(struct jffs3_inode_info),
					     0, SLAB_RECLAIM_ACCOUNT,
					     jffs3_i_init_once, NULL);
	if (!jffs3_inode_cachep) {
		ERROR_MSG("Failed to initialise inode cache\n");
		return -ENOMEM;
	}
	ret = jffs3_compressors_init();
	if (ret) {
		ERROR_MSG("Failed to initialise compressors\n");
		goto out;
	}
	ret = jffs3_create_slab_caches();
	if (ret) {
		ERROR_MSG("Failed to initialise slab caches\n");
		goto out_compressors;
	}
	ret = register_filesystem(&jffs3_fs_type);
	if (ret) {
		ERROR_MSG("Failed to register filesystem\n");
		goto out_slab;
	}
	return 0;

 out_slab:
	jffs3_destroy_slab_caches();
 out_compressors:
	jffs3_compressors_exit();
 out:
	kmem_cache_destroy(jffs3_inode_cachep);
	return ret;
}

static void __exit exit_jffs3_fs(void)
{
	unregister_filesystem(&jffs3_fs_type);
	jffs3_destroy_slab_caches();
	jffs3_compressors_exit();
	kmem_cache_destroy(jffs3_inode_cachep);
jffs3_inode_cachep = NULL;
}

module_init(init_jffs3_fs);
module_exit(exit_jffs3_fs);

MODULE_DESCRIPTION("The Journalling Flash File System, v3");
MODULE_AUTHOR("Red Hat, Inc.");
MODULE_LICENSE("GPL"); // Actually dual-licensed, but it doesn't matter for
		       // the sake of this tag. It's Free Software.

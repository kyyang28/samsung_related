/*
 * JFFS3 -- Journalling Flash File System, Version 3.
 *
 * Copyright (C) 2001-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@cambridge.redhat.com>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * JFFS2 Id: dir-ecos.c,v 1.10 2003/11/26 15:55:35 dwmw2  Exp
 * $Id: dir-ecos.c,v 3.2 2005/01/05 16:19:01 dedekind Exp $
 *
 */

#include <linux/kernel.h>
#include <linux/crc32.h>
#include "nodelist.h"

/***********************************************************************/

/* Takes length argument because it can be either NUL-terminated or '/'-terminated */
struct _inode *jffs3_lookup(struct _inode *dir_i, const unsigned char *d_name, int namelen)
{
	struct jffs3_inode_info *dir_f;
	struct jffs3_sb_info *c;
	struct jffs3_full_dirent *fd = NULL, *fd_list;
	uint32_t ino = 0;
	uint32_t hash = full_name_hash(d_name, namelen);
	struct _inode *inode = NULL;

	D1(printk("jffs3_lookup()\n"));

	dir_f = JFFS3_INODE_INFO(dir_i);
	c = JFFS3_SB_INFO(dir_i->i_sb);

	down(&dir_f->sem);

	/* NB: The 2.2 backport will need to explicitly check for '.' and '..' here */
	for (fd_list = dir_f->dents; fd_list && fd_list->nhash <= hash; fd_list = fd_list->next) {
		if (fd_list->nhash == hash && 
		    (!fd || fd_list->version > fd->version) &&
		    strlen(fd_list->name) == namelen &&
		    !strncmp(fd_list->name, d_name, namelen)) {
			fd = fd_list;
		}
	}
	if (fd)
		ino = fd->ino;
	up(&dir_f->sem);
	if (ino) {
		inode = jffs3_iget(dir_i->i_sb, ino);
		if (!inode) {
			printk("jffs3_iget() failed for ino #%u\n", ino);
			return (ERR_PTR(-EIO));
		}
	}

	return inode;
}

/***********************************************************************/



int jffs3_create(struct _inode *dir_i, const unsigned char *d_name, int mode,
                 struct _inode **new_i)
{
	struct jffs3_raw_inode *ri;
	struct jffs3_inode_info *f, *dir_f;
	struct jffs3_sb_info *c;
	struct _inode *inode;
	int ret;

	ri = jffs3_alloc_raw_inode();
	if (!ri)
		return -ENOMEM;
	
	c = JFFS3_SB_INFO(dir_i->i_sb);

	D1(printk(KERN_DEBUG "jffs3_create()\n"));

	inode = jffs3_new_inode(dir_i, mode, ri);

	if (IS_ERR(inode)) {
		D1(printk(KERN_DEBUG "jffs3_new_inode() failed\n"));
		jffs3_free_raw_inode(ri);
		return PTR_ERR(inode);
	}

	f = JFFS3_INODE_INFO(inode);
	dir_f = JFFS3_INODE_INFO(dir_i);

	ret = jffs3_do_create(c, dir_f, f, ri, 
			      d_name, strlen(d_name));

	if (ret) {
		inode->i_nlink = 0;
		jffs3_iput(inode);
		jffs3_free_raw_inode(ri);
		return ret;
	}

	jffs3_free_raw_inode(ri);

	D1(printk(KERN_DEBUG "jffs3_create: Created ino #%lu with mode %o, nlink %d(%d)\n",
		  inode->i_ino, inode->i_mode, inode->i_nlink, f->inocache->nlink));
        *new_i = inode;
	return 0;
}

/***********************************************************************/


int jffs3_unlink(struct _inode *dir_i, struct _inode *d_inode, const unsigned char *d_name)
{
	struct jffs3_sb_info *c = JFFS3_SB_INFO(dir_i->i_sb);
	struct jffs3_inode_info *dir_f = JFFS3_INODE_INFO(dir_i);
	struct jffs3_inode_info *dead_f = JFFS3_INODE_INFO(d_inode);
	int ret;

	ret = jffs3_do_unlink(c, dir_f, d_name, 
			       strlen(d_name), dead_f);
	if (dead_f->inocache)
		d_inode->i_nlink = dead_f->inocache->nlink;
	return ret;
}
/***********************************************************************/


int jffs3_link (struct _inode *old_d_inode, struct _inode *dir_i, const unsigned char *d_name)
{
	struct jffs3_sb_info *c = JFFS3_SB_INFO(old_d_inode->i_sb);
	struct jffs3_inode_info *f = JFFS3_INODE_INFO(old_d_inode);
	struct jffs3_inode_info *dir_f = JFFS3_INODE_INFO(dir_i);
	int ret;

	/* XXX: This is ugly */
	uint8_t type = (old_d_inode->i_mode & S_IFMT) >> 12;
	if (!type) type = DT_REG;

	ret = jffs3_do_link(c, dir_f, f->inocache->ino, type, d_name, strlen(d_name));

	if (!ret) {
		down(&f->sem);
		old_d_inode->i_nlink = ++f->inocache->nlink;
		up(&f->sem);
	}
	return ret;
}

int jffs3_mkdir (struct _inode *dir_i, const unsigned char *d_name, int mode)
{
	struct jffs3_inode_info *f, *dir_f;
	struct jffs3_sb_info *c;
	struct _inode *inode;
	struct jffs3_raw_inode *ri;
	struct jffs3_raw_dirent *rd;
	struct jffs3_full_dnode *fn;
	struct jffs3_full_dirent *fd;
	int namelen;
	uint32_t alloclen, phys_ofs;
	int ret;

	mode |= S_IFDIR;

	ri = jffs3_alloc_raw_inode();
	if (!ri)
		return -ENOMEM;
	
	c = JFFS3_SB_INFO(dir_i->i_sb);

	/* Try to reserve enough space for both node and dirent. 
	 * Just the node will do for now, though 
	 */
	namelen = strlen(d_name);
	ret = jffs3_reserve_space(c, sizeof(*ri), &phys_ofs, &alloclen, ALLOC_NORMAL);

	if (ret) {
		jffs3_free_raw_inode(ri);
		return ret;
	}

	inode = jffs3_new_inode(dir_i, mode, ri);

	if (IS_ERR(inode)) {
		jffs3_free_raw_inode(ri);
		jffs3_complete_reservation(c);
		return PTR_ERR(inode);
	}

	f = JFFS3_INODE_INFO(inode);

	ri->data_crc = cpu_to_je32(0);
	ri->node_crc = cpu_to_je32(crc32(0, ri, sizeof(*ri)-8));
	
	fn = jffs3_write_dnode(c, f, ri, NULL, 0, phys_ofs, ALLOC_NORMAL);

	jffs3_free_raw_inode(ri);

	if (IS_ERR(fn)) {
		/* Eeek. Wave bye bye */
		up(&f->sem);
		jffs3_complete_reservation(c);
		inode->i_nlink = 0;
		jffs3_iput(inode);
		return PTR_ERR(fn);
	}
	/* No data here. Only a metadata node, which will be 
	   obsoleted by the first data write
	*/
	f->metadata = fn;
	up(&f->sem);

	jffs3_complete_reservation(c);
	ret = jffs3_reserve_space(c, sizeof(*rd)+namelen, &phys_ofs, &alloclen, ALLOC_NORMAL);
	if (ret) {
		/* Eep. */
		inode->i_nlink = 0;
		jffs3_iput(inode);
		return ret;
	}
	
	rd = jffs3_alloc_raw_dirent();
	if (!rd) {
		/* Argh. Now we treat it like a normal delete */
		jffs3_complete_reservation(c);
		inode->i_nlink = 0;
		jffs3_iput(inode);
		return -ENOMEM;
	}

	dir_f = JFFS3_INODE_INFO(dir_i);
	down(&dir_f->sem);

	rd->magic = cpu_to_je16(JFFS3_MAGIC_BITMASK);
	rd->nodetype = cpu_to_je16(JFFS3_NODETYPE_DIRENT);
	rd->totlen = cpu_to_je32(sizeof(*rd) + namelen);
	rd->hdr_crc = cpu_to_je32(crc32(0, rd, sizeof(struct jffs3_unknown_node)-4));

	rd->pino = cpu_to_je32(dir_i->i_ino);
	rd->version = cpu_to_je32(++dir_f->highest_version);
	rd->ino = cpu_to_je32(inode->i_ino);
	rd->mctime = cpu_to_je32(cyg_timestamp());
	rd->nsize = namelen;
	rd->type = DT_DIR;
	rd->node_crc = cpu_to_je32(crc32(0, rd, sizeof(*rd)-8));
	rd->name_crc = cpu_to_je32(crc32(0, d_name, namelen));

	fd = jffs3_write_dirent(c, dir_f, rd, d_name, namelen, phys_ofs, ALLOC_NORMAL);
	
	jffs3_complete_reservation(c);
	jffs3_free_raw_dirent(rd);
	
	if (IS_ERR(fd)) {
		/* dirent failed to write. Delete the inode normally 
		   as if it were the final unlink() */
		up(&dir_f->sem);
		inode->i_nlink = 0;
		jffs3_iput(inode);
		return PTR_ERR(fd);
	}

	/* Link the fd into the inode's list, obsoleting an old
	   one if necessary. */
	jffs3_add_fd_to_list(c, fd, &dir_f->dents);
	up(&dir_f->sem);

	jffs3_iput(inode);
	return 0;
}

int jffs3_rmdir (struct _inode *dir_i, struct _inode *d_inode, const unsigned char *d_name)
{
	struct jffs3_inode_info *f = JFFS3_INODE_INFO(d_inode);
	struct jffs3_full_dirent *fd;

	for (fd = f->dents ; fd; fd = fd->next) {
		if (fd->ino)
			return EPERM; //-ENOTEMPTY;
	}
	return jffs3_unlink(dir_i, d_inode, d_name);
}

int jffs3_rename (struct _inode *old_dir_i, struct _inode *d_inode, const unsigned char *old_d_name,
		  struct _inode *new_dir_i, const unsigned char *new_d_name)
{
	int ret;
	struct jffs3_sb_info *c = JFFS3_SB_INFO(old_dir_i->i_sb);
	struct jffs3_inode_info *victim_f = NULL;
	uint8_t type;

#if 0 /* FIXME -- this really doesn't belong in individual file systems. 
	 The fileio code ought to do this for us, or at least part of it */
	if (new_dentry->d_inode) {
		if (S_ISDIR(d_inode->i_mode) && 
		    !S_ISDIR(new_dentry->d_inode->i_mode)) {
			/* Cannot rename directory over non-directory */
			return -EINVAL;
		}

		victim_f = JFFS3_INODE_INFO(new_dentry->d_inode);

		if (S_ISDIR(new_dentry->d_inode->i_mode)) {
			struct jffs3_full_dirent *fd;

			if (!S_ISDIR(d_inode->i_mode)) {
				/* Cannot rename non-directory over directory */
				return -EINVAL;
			}
			down(&victim_f->sem);
			for (fd = victim_f->dents; fd; fd = fd->next) {
				if (fd->ino) {
					up(&victim_f->sem);
					return -ENOTEMPTY;
				}
			}
			up(&victim_f->sem);
		}
	}
#endif

	/* XXX: We probably ought to alloc enough space for
	   both nodes at the same time. Writing the new link, 
	   then getting -ENOSPC, is quite bad :)
	*/

	/* Make a hard link */
	
	/* XXX: This is ugly */
	type = (d_inode->i_mode & S_IFMT) >> 12;
	if (!type) type = DT_REG;

	ret = jffs3_do_link(c, JFFS3_INODE_INFO(new_dir_i), 
			    d_inode->i_ino, type,
			    new_d_name, strlen(new_d_name));

	if (ret)
		return ret;

	if (victim_f) {
		/* There was a victim. Kill it off nicely */
		/* Don't oops if the victim was a dirent pointing to an
		   inode which didn't exist. */
		if (victim_f->inocache) {
			down(&victim_f->sem);
			victim_f->inocache->nlink--;
			up(&victim_f->sem);
		}
	}

	/* Unlink the original */
	ret = jffs3_do_unlink(c, JFFS3_INODE_INFO(old_dir_i), 
		      old_d_name, strlen(old_d_name), NULL);

	if (ret) {
		/* Oh shit. We really ought to make a single node which can do both atomically */
		struct jffs3_inode_info *f = JFFS3_INODE_INFO(d_inode);
		down(&f->sem);
		if (f->inocache)
			d_inode->i_nlink = f->inocache->nlink++;
		up(&f->sem);

		printk(KERN_NOTICE "jffs3_rename(): Link succeeded, unlink failed (err %d). You now have a hard link\n", ret);
	}
	return ret;
}


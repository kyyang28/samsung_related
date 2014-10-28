/*
 * JFFS3 -- Journalling Flash File System, Version 3.
 *
 * Copyright (C) 2001-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@infradead.org>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * JFFS2 Id: dir.c,v 1.84 2004/11/16 20:36:11 dwmw2  Exp
 * $Id: dir.c,v 3.9 2005/01/05 16:19:00 dedekind Exp $
 *
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/crc32.h>
#include <linux/time.h>
#include "jffs3.h"
#include "nodelist.h"
#include "summary.h"

/* Urgh. Please tell me there's a nicer way of doing these. */
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,48)
typedef int mknod_arg_t;
#define NAMEI_COMPAT(x) ((void *)x)
#else
typedef dev_t mknod_arg_t;
#define NAMEI_COMPAT(x) (x)
#endif

static int jffs3_readdir (struct file *, void *, filldir_t);

static int jffs3_create (struct inode *,struct dentry *,int,
			 struct nameidata *);
static struct dentry *jffs3_lookup (struct inode *,struct dentry *,
				    struct nameidata *);
static int jffs3_link (struct dentry *,struct inode *,struct dentry *);
static int jffs3_unlink (struct inode *,struct dentry *);
static int jffs3_symlink (struct inode *,struct dentry *,const char *);
static int jffs3_mkdir (struct inode *,struct dentry *,int);
static int jffs3_rmdir (struct inode *,struct dentry *);
static int jffs3_mknod (struct inode *,struct dentry *,int,mknod_arg_t);
static int jffs3_rename (struct inode *, struct dentry *,
                        struct inode *, struct dentry *);

struct file_operations jffs3_dir_operations =
{
	.read =		generic_read_dir,
	.readdir =	jffs3_readdir,
	.ioctl =	jffs3_ioctl,
	.fsync =	jffs3_fsync
};


struct inode_operations jffs3_dir_inode_operations =
{
	.create =	NAMEI_COMPAT(jffs3_create),
	.lookup =	NAMEI_COMPAT(jffs3_lookup),
	.link =		jffs3_link,
	.unlink =	jffs3_unlink,
	.symlink =	jffs3_symlink,
	.mkdir =	jffs3_mkdir,
	.rmdir =	jffs3_rmdir,
	.mknod =	jffs3_mknod,
	.rename =	jffs3_rename,
	.setattr =	jffs3_setattr,
};

/***********************************************************************/


/* We keep the dirent list sorted in increasing order of name hash,
   and we use the same hash function as the dentries. Makes this
   nice and simple
*/
static struct dentry *jffs3_lookup(struct inode *dir_i, struct dentry *target,
				   struct nameidata *nd)
{
	struct jffs3_inode_info *dir_f;
	struct jffs3_sb_info *c;
	struct jffs3_full_dirent *fd = NULL, *fd_list;
	uint32_t ino = 0;
	struct inode *inode = NULL;

	DBG_VFS(1, "entering\n");

	dir_f = JFFS3_INODE_INFO(dir_i);
	c = JFFS3_SB_INFO(dir_i->i_sb);

	down(&dir_f->sem);

	/* NB: The 2.2 backport will need to explicitly check for '.' and '..' here */
	for (fd_list = dir_f->dents; fd_list && fd_list->nhash <= target->d_name.hash; fd_list = fd_list->next) {
		if (fd_list->nhash == target->d_name.hash &&
		    (!fd || fd_list->version > fd->version) &&
		    strlen(fd_list->name) == target->d_name.len &&
		    !strncmp(fd_list->name, target->d_name.name, target->d_name.len)) {
			fd = fd_list;
		}
	}
	if (fd)
		ino = fd->ino;
	up(&dir_f->sem);
	if (ino) {
		inode = iget(dir_i->i_sb, ino);
		if (!inode) {
			WARNING_MSG("iget() failed for ino #%u\n", ino);
			return (ERR_PTR(-EIO));
		}
	}

	d_add(target, inode);

	return NULL;
}

/***********************************************************************/


static int jffs3_readdir(struct file *filp, void *dirent, filldir_t filldir)
{
	struct jffs3_inode_info *f;
	struct jffs3_sb_info *c;
	struct inode *inode = filp->f_dentry->d_inode;
	struct jffs3_full_dirent *fd;
	unsigned long offset, curofs;

	DBG_VFS(1, "for dir_i #%lu\n", filp->f_dentry->d_inode->i_ino);

	f = JFFS3_INODE_INFO(inode);
	c = JFFS3_SB_INFO(inode->i_sb);

	offset = filp->f_pos;

	if (offset == 0) {
		DBG_VFS(1, "Dirent 0: \".\", ino #%lu\n", inode->i_ino);
		if (filldir(dirent, ".", 1, 0, inode->i_ino, DT_DIR) < 0)
			goto out;
		offset++;
	}
	if (offset == 1) {
		unsigned long pino = parent_ino(filp->f_dentry);
		DBG_VFS(1, "Dirent 1: \"..\", ino #%lu\n", pino);
		if (filldir(dirent, "..", 2, 1, pino, DT_DIR) < 0)
			goto out;
		offset++;
	}

	curofs=1;
	down(&f->sem);
	for (fd = f->dents; fd; fd = fd->next) {

		curofs++;
		/* First loop: curofs = 2; offset = 2 */
		if (curofs < offset) {
			DBG_VFS(2, "Skipping dirent: \"%s\", ino #%u, type %d, because curofs %ld < offset %ld\n",
				  fd->name, fd->ino, fd->type, curofs, offset);
			continue;
		}
		if (!fd->ino) {
			DBG_VFS(2, "Skipping deletion dirent \"%s\"\n", fd->name);
			offset++;
			continue;
		}
		DBG_VFS(2, "Dirent %ld: \"%s\", ino #%u, type %d\n", offset, fd->name, fd->ino, fd->type);
		if (filldir(dirent, fd->name, strlen(fd->name), offset, fd->ino, fd->type) < 0)
			break;
		offset++;
	}
	up(&f->sem);
 out:
	filp->f_pos = offset;
	return 0;
}

/***********************************************************************/


static int jffs3_create(struct inode *dir_i, struct dentry *dentry, int mode,
			struct nameidata *nd)
{
	struct jffs3_raw_inode *ri;
	struct jffs3_inode_info *f, *dir_f;
	struct jffs3_sb_info *c;
	struct inode *inode;
	int ret;

	DBG_VFS(1, "entering\n");
	ri = jffs3_alloc_raw_inode();
	if (!ri)
		return -ENOMEM;

	c = JFFS3_SB_INFO(dir_i->i_sb);

	inode = jffs3_new_inode(dir_i, mode, ri);

	if (IS_ERR(inode)) {
		DBG_VFS(1, "failed\n");
		jffs3_free_raw_inode(ri);
		return PTR_ERR(inode);
	}

	inode->i_op = &jffs3_file_inode_operations;
	inode->i_fop = &jffs3_file_operations;
	inode->i_mapping->a_ops = &jffs3_file_address_operations;
	inode->i_mapping->nrpages = 0;

	f = JFFS3_INODE_INFO(inode);
	dir_f = JFFS3_INODE_INFO(dir_i);

	ret = jffs3_do_create(c, dir_f, f, ri,
			      dentry->d_name.name, dentry->d_name.len);

	if (ret) {
		make_bad_inode(inode);
		iput(inode);
		jffs3_free_raw_inode(ri);
		return ret;
	}

	dir_i->i_mtime = dir_i->i_ctime = ITIME(je32_to_cpu(ri->ctime));

	jffs3_free_raw_inode(ri);
	d_instantiate(dentry, inode);

	DBG_VFS(1, "Created ino #%lu with mode %o, nlink %d(%d). nrpages %ld\n",
		  inode->i_ino, inode->i_mode, inode->i_nlink,
		  f->inocache->nlink, inode->i_mapping->nrpages);
	return 0;
}

/***********************************************************************/


static int jffs3_unlink(struct inode *dir_i, struct dentry *dentry)
{
	struct jffs3_sb_info *c = JFFS3_SB_INFO(dir_i->i_sb);
	struct jffs3_inode_info *dir_f = JFFS3_INODE_INFO(dir_i);
	struct jffs3_inode_info *dead_f = JFFS3_INODE_INFO(dentry->d_inode);
	int ret;

	DBG_VFS(1, "entering\n");
	ret = jffs3_do_unlink(c, dir_f, dentry->d_name.name,
			       dentry->d_name.len, dead_f);
	if (dead_f->inocache)
		dentry->d_inode->i_nlink = dead_f->inocache->nlink;
	return ret;
}
/***********************************************************************/


static int jffs3_link (struct dentry *old_dentry, struct inode *dir_i, struct dentry *dentry)
{
	struct jffs3_sb_info *c = JFFS3_SB_INFO(old_dentry->d_inode->i_sb);
	struct jffs3_inode_info *f = JFFS3_INODE_INFO(old_dentry->d_inode);
	struct jffs3_inode_info *dir_f = JFFS3_INODE_INFO(dir_i);
	int ret;
	uint8_t type;

	DBG_VFS(1, "entering\n");
	/* Don't let people make hard links to bad inodes. */
	if (!f->inocache)
		return -EIO;

	if (S_ISDIR(old_dentry->d_inode->i_mode))
		return -EPERM;

	/* XXX: This is ugly */
	type = (old_dentry->d_inode->i_mode & S_IFMT) >> 12;
	if (!type) type = DT_REG;

	ret = jffs3_do_link(c, dir_f, f->inocache->ino, type, dentry->d_name.name, dentry->d_name.len);

	if (!ret) {
		down(&f->sem);
		old_dentry->d_inode->i_nlink = ++f->inocache->nlink;
		up(&f->sem);
		d_instantiate(dentry, old_dentry->d_inode);
		atomic_inc(&old_dentry->d_inode->i_count);
	}
	return ret;
}

/***********************************************************************/

static int jffs3_symlink (struct inode *dir_i, struct dentry *dentry, const char *target)
{
	struct jffs3_inode_info *f, *dir_f;
	struct jffs3_sb_info *c;
	struct inode *inode;
	struct jffs3_raw_inode *ri;
	struct jffs3_raw_dirent *rd;
	struct jffs3_full_dnode *fn;
	struct jffs3_full_dirent *fd;
	int namelen;
	uint32_t alloclen, phys_ofs;
	int ret;

	DBG_VFS(1, "entering\n");
	/* FIXME: If you care. We'd need to use frags for the target
	   if it grows much more than this */
	if (strlen(target) > 254)
		return -EINVAL;

	ri = jffs3_alloc_raw_inode();

	if (!ri)
		return -ENOMEM;

	c = JFFS3_SB_INFO(dir_i->i_sb);

	/* Try to reserve enough space for both node and dirent.
	 * Just the node will do for now, though
	 */
	namelen = dentry->d_name.len;
	ret = jffs3_reserve_space(c, sizeof(*ri) + strlen(target), &phys_ofs, &alloclen, ALLOC_NORMAL,  JFFS3_SUMMARY_INODE_SIZE);

	if (ret) {
		jffs3_free_raw_inode(ri);
		return ret;
	}

	inode = jffs3_new_inode(dir_i, S_IFLNK | S_IRWXUGO, ri);

	if (IS_ERR(inode)) {
		jffs3_free_raw_inode(ri);
		jffs3_complete_reservation(c);
		return PTR_ERR(inode);
	}

	inode->i_op = &jffs3_symlink_inode_operations;

	f = JFFS3_INODE_INFO(inode);

	inode->i_size = strlen(target);
	ri->isize = ri->dsize = ri->csize = cpu_to_je32(inode->i_size);
	ri->totlen = cpu_to_je32(sizeof(*ri) + inode->i_size);
	ri->hdr_crc = cpu_to_je32(crc32(0, ri, sizeof(struct jffs3_unknown_node)-4));

	ri->compr = JFFS3_COMPR_NONE;
	ri->data_crc = cpu_to_je32(crc32(0, target, strlen(target)));
	ri->node_crc = cpu_to_je32(crc32(0, ri, sizeof(*ri)-8));

	fn = jffs3_write_dnode(c, f, ri, target, strlen(target), phys_ofs, ALLOC_NORMAL);

	jffs3_free_raw_inode(ri);

	if (IS_ERR(fn)) {
		/* Eeek. Wave bye bye */
		up(&f->sem);
		jffs3_complete_reservation(c);
		jffs3_clear_inode(inode);
		return PTR_ERR(fn);
	}
	/* No data here. Only a metadata node, which will be
	   obsoleted by the first data write
	*/
	f->metadata = fn;
	up(&f->sem);

	jffs3_complete_reservation(c);
	ret = jffs3_reserve_space(c, sizeof(*rd)+namelen, &phys_ofs, &alloclen, ALLOC_NORMAL, JFFS3_SUMMARY_DIRENT_SIZE(namelen));
	if (ret) {
		/* Eep. */
		jffs3_clear_inode(inode);
		return ret;
	}

	rd = jffs3_alloc_raw_dirent();
	if (!rd) {
		/* Argh. Now we treat it like a normal delete */
		jffs3_complete_reservation(c);
		jffs3_clear_inode(inode);
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
	rd->mctime = cpu_to_je32(get_seconds());
	rd->nsize = namelen;
	rd->type = DT_LNK;
	rd->node_crc = cpu_to_je32(crc32(0, rd, sizeof(*rd)-8));
	rd->name_crc = cpu_to_je32(crc32(0, dentry->d_name.name, namelen));

	fd = jffs3_write_dirent(c, dir_f, rd, dentry->d_name.name, namelen, phys_ofs, ALLOC_NORMAL);

	if (IS_ERR(fd)) {
		/* dirent failed to write. Delete the inode normally
		   as if it were the final unlink() */
		jffs3_complete_reservation(c);
		jffs3_free_raw_dirent(rd);
		up(&dir_f->sem);
		jffs3_clear_inode(inode);
		return PTR_ERR(fd);
	}

	dir_i->i_mtime = dir_i->i_ctime = ITIME(je32_to_cpu(rd->mctime));

	jffs3_free_raw_dirent(rd);

	/* Link the fd into the inode's list, obsoleting an old
	   one if necessary. */
	jffs3_add_fd_to_list(c, fd, &dir_f->dents);

	up(&dir_f->sem);
	jffs3_complete_reservation(c);

	d_instantiate(dentry, inode);
	return 0;
}


static int jffs3_mkdir (struct inode *dir_i, struct dentry *dentry, int mode)
{
	struct jffs3_inode_info *f, *dir_f;
	struct jffs3_sb_info *c;
	struct inode *inode;
	struct jffs3_raw_inode *ri;
	struct jffs3_raw_dirent *rd;
	struct jffs3_full_dnode *fn;
	struct jffs3_full_dirent *fd;
	int namelen;
	uint32_t alloclen, phys_ofs;
	int ret;

	mode |= S_IFDIR;

	DBG_VFS(1, "entering\n");
	ri = jffs3_alloc_raw_inode();
	if (!ri)
		return -ENOMEM;

	c = JFFS3_SB_INFO(dir_i->i_sb);

	/* Try to reserve enough space for both node and dirent.
	 * Just the node will do for now, though
	 */
	namelen = dentry->d_name.len;
	ret = jffs3_reserve_space(c, sizeof(*ri), &phys_ofs, &alloclen, ALLOC_NORMAL, JFFS3_SUMMARY_INODE_SIZE);

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

	inode->i_op = &jffs3_dir_inode_operations;
	inode->i_fop = &jffs3_dir_operations;
	/* Directories get nlink 2 at start */
	inode->i_nlink = 2;

	f = JFFS3_INODE_INFO(inode);

	ri->data_crc = cpu_to_je32(0);
	ri->node_crc = cpu_to_je32(crc32(0, ri, sizeof(*ri)-8));

	fn = jffs3_write_dnode(c, f, ri, NULL, 0, phys_ofs, ALLOC_NORMAL);

	jffs3_free_raw_inode(ri);

	if (IS_ERR(fn)) {
		/* Eeek. Wave bye bye */
		up(&f->sem);
		jffs3_complete_reservation(c);
		jffs3_clear_inode(inode);
		return PTR_ERR(fn);
	}
	/* No data here. Only a metadata node, which will be
	   obsoleted by the first data write
	*/
	f->metadata = fn;
	up(&f->sem);

	jffs3_complete_reservation(c);
	ret = jffs3_reserve_space(c, sizeof(*rd)+namelen, &phys_ofs, &alloclen, ALLOC_NORMAL, JFFS3_SUMMARY_DIRENT_SIZE(namelen));
	if (ret) {
		/* Eep. */
		jffs3_clear_inode(inode);
		return ret;
	}

	rd = jffs3_alloc_raw_dirent();
	if (!rd) {
		/* Argh. Now we treat it like a normal delete */
		jffs3_complete_reservation(c);
		jffs3_clear_inode(inode);
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
	rd->mctime = cpu_to_je32(get_seconds());
	rd->nsize = namelen;
	rd->type = DT_DIR;
	rd->node_crc = cpu_to_je32(crc32(0, rd, sizeof(*rd)-8));
	rd->name_crc = cpu_to_je32(crc32(0, dentry->d_name.name, namelen));

	fd = jffs3_write_dirent(c, dir_f, rd, dentry->d_name.name, namelen, phys_ofs, ALLOC_NORMAL);

	if (IS_ERR(fd)) {
		/* dirent failed to write. Delete the inode normally
		   as if it were the final unlink() */
		jffs3_complete_reservation(c);
		jffs3_free_raw_dirent(rd);
		up(&dir_f->sem);
		jffs3_clear_inode(inode);
		return PTR_ERR(fd);
	}

	dir_i->i_mtime = dir_i->i_ctime = ITIME(je32_to_cpu(rd->mctime));
	dir_i->i_nlink++;

	jffs3_free_raw_dirent(rd);

	/* Link the fd into the inode's list, obsoleting an old
	   one if necessary. */
	jffs3_add_fd_to_list(c, fd, &dir_f->dents);

	up(&dir_f->sem);
	jffs3_complete_reservation(c);

	d_instantiate(dentry, inode);
	return 0;
}

static int jffs3_rmdir (struct inode *dir_i, struct dentry *dentry)
{
	struct jffs3_inode_info *f = JFFS3_INODE_INFO(dentry->d_inode);
	struct jffs3_full_dirent *fd;
	int ret;

	DBG_VFS(1, "entering\n");
	for (fd = f->dents ; fd; fd = fd->next) {
		if (fd->ino)
			return -ENOTEMPTY;
	}
	ret = jffs3_unlink(dir_i, dentry);
	if (!ret)
		dir_i->i_nlink--;
	return ret;
}

static int jffs3_mknod (struct inode *dir_i, struct dentry *dentry, int mode, mknod_arg_t rdev)
{
	struct jffs3_inode_info *f, *dir_f;
	struct jffs3_sb_info *c;
	struct inode *inode;
	struct jffs3_raw_inode *ri;
	struct jffs3_raw_dirent *rd;
	struct jffs3_full_dnode *fn;
	struct jffs3_full_dirent *fd;
	int namelen;
	jint16_t dev;
	int devlen = 0;
	uint32_t alloclen, phys_ofs;
	int ret;

	DBG_VFS(1, "entering\n");
	if (!old_valid_dev(rdev))
		return -EINVAL;

	ri = jffs3_alloc_raw_inode();
	if (!ri)
		return -ENOMEM;

	c = JFFS3_SB_INFO(dir_i->i_sb);

	if (S_ISBLK(mode) || S_ISCHR(mode)) {
		dev = cpu_to_je16(old_encode_dev(rdev));
		devlen = sizeof(dev);
	}

	/* Try to reserve enough space for both node and dirent.
	 * Just the node will do for now, though
	 */
	namelen = dentry->d_name.len;
	ret = jffs3_reserve_space(c, sizeof(*ri) + devlen, &phys_ofs, &alloclen, ALLOC_NORMAL, JFFS3_SUMMARY_INODE_SIZE);

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
	inode->i_op = &jffs3_file_inode_operations;
	init_special_inode(inode, inode->i_mode, rdev);

	f = JFFS3_INODE_INFO(inode);

	ri->dsize = ri->csize = cpu_to_je32(devlen);
	ri->totlen = cpu_to_je32(sizeof(*ri) + devlen);
	ri->hdr_crc = cpu_to_je32(crc32(0, ri, sizeof(struct jffs3_unknown_node)-4));

	ri->compr = JFFS3_COMPR_NONE;
	ri->data_crc = cpu_to_je32(crc32(0, &dev, devlen));
	ri->node_crc = cpu_to_je32(crc32(0, ri, sizeof(*ri)-8));

	fn = jffs3_write_dnode(c, f, ri, (char *)&dev, devlen, phys_ofs, ALLOC_NORMAL);

	jffs3_free_raw_inode(ri);

	if (IS_ERR(fn)) {
		/* Eeek. Wave bye bye */
		up(&f->sem);
		jffs3_complete_reservation(c);
		jffs3_clear_inode(inode);
		return PTR_ERR(fn);
	}
	/* No data here. Only a metadata node, which will be
	   obsoleted by the first data write
	*/
	f->metadata = fn;
	up(&f->sem);

	jffs3_complete_reservation(c);
	ret = jffs3_reserve_space(c, sizeof(*rd)+namelen, &phys_ofs, &alloclen, ALLOC_NORMAL, JFFS3_SUMMARY_DIRENT_SIZE(namelen));
	if (ret) {
		/* Eep. */
		jffs3_clear_inode(inode);
		return ret;
	}

	rd = jffs3_alloc_raw_dirent();
	if (!rd) {
		/* Argh. Now we treat it like a normal delete */
		jffs3_complete_reservation(c);
		jffs3_clear_inode(inode);
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
	rd->mctime = cpu_to_je32(get_seconds());
	rd->nsize = namelen;

	/* XXX: This is ugly. */
	rd->type = (mode & S_IFMT) >> 12;

	rd->node_crc = cpu_to_je32(crc32(0, rd, sizeof(*rd)-8));
	rd->name_crc = cpu_to_je32(crc32(0, dentry->d_name.name, namelen));

	fd = jffs3_write_dirent(c, dir_f, rd, dentry->d_name.name, namelen, phys_ofs, ALLOC_NORMAL);

	if (IS_ERR(fd)) {
		/* dirent failed to write. Delete the inode normally
		   as if it were the final unlink() */
		jffs3_complete_reservation(c);
		jffs3_free_raw_dirent(rd);
		up(&dir_f->sem);
		jffs3_clear_inode(inode);
		return PTR_ERR(fd);
	}

	dir_i->i_mtime = dir_i->i_ctime = ITIME(je32_to_cpu(rd->mctime));

	jffs3_free_raw_dirent(rd);

	/* Link the fd into the inode's list, obsoleting an old
	   one if necessary. */
	jffs3_add_fd_to_list(c, fd, &dir_f->dents);

	up(&dir_f->sem);
	jffs3_complete_reservation(c);

	d_instantiate(dentry, inode);

	return 0;
}

static int jffs3_rename (struct inode *old_dir_i, struct dentry *old_dentry,
                        struct inode *new_dir_i, struct dentry *new_dentry)
{
	int ret;
	struct jffs3_sb_info *c = JFFS3_SB_INFO(old_dir_i->i_sb);
	struct jffs3_inode_info *victim_f = NULL;
	uint8_t type;

	/* The VFS will check for us and prevent trying to rename a
	 * file over a directory and vice versa, but if it's a directory,
	 * the VFS can't check whether the victim is empty. The filesystem
	 * needs to do that for itself.
	 */
	DBG_VFS(1, "entering\n");
	if (new_dentry->d_inode) {
		victim_f = JFFS3_INODE_INFO(new_dentry->d_inode);
		if (S_ISDIR(new_dentry->d_inode->i_mode)) {
			struct jffs3_full_dirent *fd;

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

	/* XXX: We probably ought to alloc enough space for
	   both nodes at the same time. Writing the new link,
	   then getting -ENOSPC, is quite bad :)
	*/

	/* Make a hard link */

	/* XXX: This is ugly */
	type = (old_dentry->d_inode->i_mode & S_IFMT) >> 12;
	if (!type) type = DT_REG;

	ret = jffs3_do_link(c, JFFS3_INODE_INFO(new_dir_i),
			    old_dentry->d_inode->i_ino, type,
			    new_dentry->d_name.name, new_dentry->d_name.len);

	if (ret)
		return ret;

	if (victim_f) {
		/* There was a victim. Kill it off nicely */
		new_dentry->d_inode->i_nlink--;
		/* Don't oops if the victim was a dirent pointing to an
		   inode which didn't exist. */
		if (victim_f->inocache) {
			down(&victim_f->sem);
			victim_f->inocache->nlink--;
			up(&victim_f->sem);
		}
	}

	/* If it was a directory we moved, and there was no victim,
	   increase i_nlink on its new parent */
	if (S_ISDIR(old_dentry->d_inode->i_mode) && !victim_f)
		new_dir_i->i_nlink++;

	/* Unlink the original */
	ret = jffs3_do_unlink(c, JFFS3_INODE_INFO(old_dir_i),
		      old_dentry->d_name.name, old_dentry->d_name.len, NULL);

	/* We don't touch inode->i_nlink */

	if (ret) {
		/* Oh shit. We really ought to make a single node which can do both atomically */
		struct jffs3_inode_info *f = JFFS3_INODE_INFO(old_dentry->d_inode);
		down(&f->sem);
		old_dentry->d_inode->i_nlink++;
		if (f->inocache)
			f->inocache->nlink++;
		up(&f->sem);

		NOTICE_MSG("(rename): Link succeeded, unlink failed (err %d). You now have a hard link\n", ret);
		/* Might as well let the VFS know */
		d_instantiate(new_dentry, old_dentry->d_inode);
		atomic_inc(&old_dentry->d_inode->i_count);
		return ret;
	}

	if (S_ISDIR(old_dentry->d_inode->i_mode))
		old_dir_i->i_nlink--;

	return 0;
}

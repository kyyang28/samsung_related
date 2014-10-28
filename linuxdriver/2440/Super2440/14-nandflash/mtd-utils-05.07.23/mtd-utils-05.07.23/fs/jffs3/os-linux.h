/*
 * JFFS3 -- Journalling Flash File System, Version 3.
 *
 * Copyright (C) 2002-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@infradead.org>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * JFFS2 Id: os-linux.h,v 1.51 2004/11/16 20:36:11 dwmw2  Exp
 * $Id: os-linux.h,v 3.6 2005/02/09 14:52:05 pavlov Exp $
 *
 */

#ifndef __JFFS3_OS_LINUX_H__
#define __JFFS3_OS_LINUX_H__
#include <linux/version.h>

/* JFFS3 uses Linux mode bits natively -- no need for conversion */
#define os_to_jffs3_mode(x) (x)
#define jffs3_to_os_mode(x) (x)

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,73)
#define kstatfs statfs
#endif

struct kstatfs;
struct kvec;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,5,2)
#define JFFS3_INODE_INFO(i) (list_entry(i, struct jffs3_inode_info, vfs_inode))
#define OFNI_EDONI_2SFFJ(f)  (&(f)->vfs_inode)
#define JFFS3_SB_INFO(sb) (sb->s_fs_info)
#define OFNI_BS_2SFFJ(c)  ((struct super_block *)c->os_priv)
#elif defined(JFFS3_OUT_OF_KERNEL)
#define JFFS3_INODE_INFO(i) ((struct jffs3_inode_info *) &(i)->u)
#define OFNI_EDONI_2SFFJ(f)  ((struct inode *) ( ((char *)f) - ((char *)(&((struct inode *)NULL)->u)) ) )
#define JFFS3_SB_INFO(sb) ((struct jffs3_sb_info *) &(sb)->u)
#define OFNI_BS_2SFFJ(c)  ((struct super_block *) ( ((char *)c) - ((char *)(&((struct super_block *)NULL)->u)) ) )
#else
#define JFFS3_INODE_INFO(i) (&i->u.jffs3_i)
#define OFNI_EDONI_2SFFJ(f)  ((struct inode *) ( ((char *)f) - ((char *)(&((struct inode *)NULL)->u)) ) )
#define JFFS3_SB_INFO(sb) (&sb->u.jffs3_sb)
#define OFNI_BS_2SFFJ(c)  ((struct super_block *) ( ((char *)c) - ((char *)(&((struct super_block *)NULL)->u)) ) )
#endif


#define JFFS3_F_I_SIZE(f) (OFNI_EDONI_2SFFJ(f)->i_size)
#define JFFS3_F_I_MODE(f) (OFNI_EDONI_2SFFJ(f)->i_mode)
#define JFFS3_F_I_UID(f) (OFNI_EDONI_2SFFJ(f)->i_uid)
#define JFFS3_F_I_GID(f) (OFNI_EDONI_2SFFJ(f)->i_gid)

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,5,1)
#define JFFS3_F_I_RDEV_MIN(f) (iminor(OFNI_EDONI_2SFFJ(f)))
#define JFFS3_F_I_RDEV_MAJ(f) (imajor(OFNI_EDONI_2SFFJ(f)))
#else
#define JFFS3_F_I_RDEV_MIN(f) (MINOR(to_kdev_t(OFNI_EDONI_2SFFJ(f)->i_rdev)))
#define JFFS3_F_I_RDEV_MAJ(f) (MAJOR(to_kdev_t(OFNI_EDONI_2SFFJ(f)->i_rdev)))
#endif

/* Urgh. The things we do to keep the 2.4 build working */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,5,47)
#define ITIME(sec) ((struct timespec){sec, 0})
#define I_SEC(tv) ((tv).tv_sec)
#define JFFS3_F_I_CTIME(f) (OFNI_EDONI_2SFFJ(f)->i_ctime.tv_sec)
#define JFFS3_F_I_MTIME(f) (OFNI_EDONI_2SFFJ(f)->i_mtime.tv_sec)
#define JFFS3_F_I_ATIME(f) (OFNI_EDONI_2SFFJ(f)->i_atime.tv_sec)
#else
#define ITIME(x) (x)
#define I_SEC(x) (x)
#define JFFS3_F_I_CTIME(f) (OFNI_EDONI_2SFFJ(f)->i_ctime)
#define JFFS3_F_I_MTIME(f) (OFNI_EDONI_2SFFJ(f)->i_mtime)
#define JFFS3_F_I_ATIME(f) (OFNI_EDONI_2SFFJ(f)->i_atime)
#endif

#define sleep_on_spinunlock(wq, s)				\
	do {							\
		DECLARE_WAITQUEUE(__wait, current);		\
		add_wait_queue((wq), &__wait);			\
		set_current_state(TASK_UNINTERRUPTIBLE);	\
		spin_unlock(s);					\
		schedule();					\
		remove_wait_queue((wq), &__wait);		\
	} while(0)

static inline void jffs3_init_inode_info(struct jffs3_inode_info *f)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,5,2)
	f->highest_version = 0;
	f->fragtree = RB_ROOT;
	f->metadata = NULL;
	f->dents = NULL;
	f->flags = 0;
	f->usercompr = 0;
#else
	memset(f, 0, sizeof(*f));
	init_MUTEX_LOCKED(&f->sem);
#endif
}

#define SECTOR_ADDR(x) ( ((unsigned long)(x) & ~(c->sector_size-1)) )

#define jffs3_is_readonly(c) (OFNI_BS_2SFFJ(c)->s_flags & MS_RDONLY)
#define jffs3_is_writebuffered(c) (c->wbuf != NULL)

#ifndef CONFIG_JFFS3_FS_WRITEBUFFER
#ifndef CONFIG_JFFS3_SUMMARY
#define jffs3_can_mark_obsolete(c) (1)
#else
#define jffs3_can_mark_obsolete(c) (0)
#endif

#define jffs3_cleanmarker_oob(c) (0)
#define jffs3_write_nand_cleanmarker(c,jeb) (-EIO)

#define jffs3_flash_write(c, ofs, len, retlen, buf) ((c)->mtd->write((c)->mtd, ofs, len, retlen, buf))
#define jffs3_flash_read(c, ofs, len, retlen, buf) ((c)->mtd->read((c)->mtd, ofs, len, retlen, buf))
#define jffs3_flush_wbuf_pad(c) ({ (void)(c), 0; })
#define jffs3_flush_wbuf_gc(c, i) ({ (void)(c), (void) i, 0; })
#define jffs3_write_nand_badblock(c,jeb,bad_offset) (1)
#define jffs3_nand_flash_setup(c) (0)
#define jffs3_nand_flash_cleanup(c) do {} while(0)
#define jffs3_wbuf_dirty(c) (0)
#define jffs3_flash_writev(a,b,c,d,e,f) jffs3_flash_direct_writev(a,b,c,d,e)
#define jffs3_wbuf_timeout NULL
#define jffs3_wbuf_process NULL
#define jffs3_nor_ecc(c) (0)
#define jffs3_nor_ecc_flash_setup(c) (0)
#define jffs3_nor_ecc_flash_cleanup(c) do {} while (0)

#else /* NAND and/or ECC'd NOR support present */

#ifndef CONFIG_JFFS3_SUMMARY
#define jffs3_can_mark_obsolete(c) ((c->mtd->type == MTD_NORFLASH && !(c->mtd->flags & MTD_ECC)) || c->mtd->type == MTD_RAM)
#else
#define jffs3_can_mark_obsolete(c) (0)
#endif

#define jffs3_cleanmarker_oob(c) (c->mtd->type == MTD_NANDFLASH)

#define jffs3_flash_write_oob(c, ofs, len, retlen, buf) ((c)->mtd->write_oob((c)->mtd, ofs, len, retlen, buf))
#define jffs3_flash_read_oob(c, ofs, len, retlen, buf) ((c)->mtd->read_oob((c)->mtd, ofs, len, retlen, buf))
#define jffs3_wbuf_dirty(c) (!!(c)->wbuf_len)

/* wbuf.c */
int jffs3_flash_writev(struct jffs3_sb_info *c, const struct kvec *vecs, unsigned long count, loff_t to, size_t *retlen, uint32_t ino);
int jffs3_flash_write(struct jffs3_sb_info *c, loff_t ofs, size_t len, size_t *retlen, const u_char *buf);
int jffs3_flash_read(struct jffs3_sb_info *c, loff_t ofs, size_t len, size_t *retlen, u_char *buf);
int jffs3_check_oob_empty(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb,int mode);
int jffs3_check_nand_cleanmarker(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb);
int jffs3_write_nand_cleanmarker(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb);
int jffs3_write_nand_badblock(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb, uint32_t bad_offset);
void jffs3_wbuf_timeout(unsigned long data);
void jffs3_wbuf_process(void *data);
int jffs3_flush_wbuf_gc(struct jffs3_sb_info *c, uint32_t ino);
int jffs3_flush_wbuf_pad(struct jffs3_sb_info *c);
int jffs3_nand_flash_setup(struct jffs3_sb_info *c);
void jffs3_nand_flash_cleanup(struct jffs3_sb_info *c);

#define jffs3_nor_ecc(c) (c->mtd->type == MTD_NORFLASH && (c->mtd->flags & MTD_ECC))
int jffs3_nor_ecc_flash_setup(struct jffs3_sb_info *c);
void jffs3_nor_ecc_flash_cleanup(struct jffs3_sb_info *c);

#endif /* WRITEBUFFER */

/* erase.c */
static inline void jffs3_erase_pending_trigger(struct jffs3_sb_info *c)
{
	OFNI_BS_2SFFJ(c)->s_dirt = 1;
}

/* background.c */
int jffs3_start_garbage_collect_thread(struct jffs3_sb_info *c);
void jffs3_stop_garbage_collect_thread(struct jffs3_sb_info *c);
void jffs3_garbage_collect_trigger(struct jffs3_sb_info *c);

/* dir.c */
extern struct file_operations jffs3_dir_operations;
extern struct inode_operations jffs3_dir_inode_operations;

/* file.c */
extern struct file_operations jffs3_file_operations;
extern struct inode_operations jffs3_file_inode_operations;
extern struct address_space_operations jffs3_file_address_operations;
int jffs3_fsync(struct file *, struct dentry *, int);
int jffs3_do_readpage_nolock (struct inode *inode, struct page *pg);
int jffs3_do_readpage_unlock (struct inode *inode, struct page *pg);
int jffs3_readpage (struct file *, struct page *);
int jffs3_prepare_write (struct file *, struct page *, unsigned, unsigned);
int jffs3_commit_write (struct file *, struct page *, unsigned, unsigned);

/* ioctl.c */
int jffs3_ioctl(struct inode *, struct file *, unsigned int, unsigned long);

/* symlink.c */
extern struct inode_operations jffs3_symlink_inode_operations;

/* fs.c */
int jffs3_setattr (struct dentry *, struct iattr *);
void jffs3_read_inode (struct inode *);
void jffs3_clear_inode (struct inode *);
void jffs3_dirty_inode(struct inode *inode);
struct inode *jffs3_new_inode (struct inode *dir_i, int mode,
			       struct jffs3_raw_inode *ri);
int jffs3_statfs (struct super_block *, struct kstatfs *);
void jffs3_write_super (struct super_block *);
int jffs3_remount_fs (struct super_block *, int *, char *);
int jffs3_do_fill_super(struct super_block *sb, void *data, int silent);
void jffs3_gc_release_inode(struct jffs3_sb_info *c,
			    struct jffs3_inode_info *f);
struct jffs3_inode_info *jffs3_gc_fetch_inode(struct jffs3_sb_info *c,
					      int inum, int nlink);

unsigned char *jffs3_gc_fetch_page(struct jffs3_sb_info *c,
				   struct jffs3_inode_info *f,
				   unsigned long offset,
				   unsigned long *priv);
void jffs3_gc_release_page(struct jffs3_sb_info *c,
			   unsigned char *pg,
			   unsigned long *priv);
int jffs3_flash_setup(struct jffs3_sb_info *c);
void jffs3_flash_cleanup(struct jffs3_sb_info *c);


/* writev.c */
int jffs3_flash_direct_writev(struct jffs3_sb_info *c, const struct kvec *vecs,
		       unsigned long count, loff_t to, size_t *retlen);


#endif /* __JFFS3_OS_LINUX_H__ */

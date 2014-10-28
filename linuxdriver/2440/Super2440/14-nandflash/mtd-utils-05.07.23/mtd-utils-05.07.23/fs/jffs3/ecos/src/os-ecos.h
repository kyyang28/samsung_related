/*
 * JFFS3 -- Journalling Flash File System, Version 3.
 *
 * Copyright (C) 2002-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@cambridge.redhat.com>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * JFFS2 Id: os-ecos.h,v 1.22 2004/11/12 16:49:18 lunn  Exp
 * $Id: os-ecos.h,v 3.3 2005/02/09 14:25:16 pavlov Exp $
 *
 */

#ifndef __JFFS3_OS_ECOS_H__
#define __JFFS3_OS_ECOS_H__

#include <pkgconf/fs_jffs3.h>
#include <cyg/io/io.h>
#include <sys/types.h>
#include <asm/atomic.h>
#include <linux/stat.h>
#include <linux/compiler.h>

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <pkgconf/io_fileio.h>

#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

#include <stdlib.h>
#include <string.h>

#include <cyg/fileio/fileio.h>

#include <cyg/hal/drv_api.h>
#include <cyg/infra/diag.h>

#include <cyg/io/flash.h>

#include <linux/types.h>
#include <linux/list.h>
#include <asm/bug.h>

#define printf diag_printf

struct _inode;
struct super_block;

struct iovec {
        void *iov_base;
        ssize_t iov_len; 
};

static inline unsigned int full_name_hash(const unsigned char * name, unsigned int len) {

	unsigned hash = 0;
 	while (len--) {
		hash = (hash << 4) | (hash >> 28);
		hash ^= *(name++);
	}
	return hash;
}

#ifdef CYGOPT_FS_JFFS3_WRITE
#define jffs3_is_readonly(c) (0)
#else
#define jffs3_is_readonly(c) (1)
#endif

/* NAND flash not currently supported on eCos */
#define jffs3_can_mark_obsolete(c) (1)

#define JFFS3_INODE_INFO(i) (&(i)->jffs3_i)
#define OFNI_EDONI_2SFFJ(f)  ((struct _inode *) ( ((char *)f) - ((char *)(&((struct _inode *)NULL)->jffs3_i)) ) )
 
#define JFFS3_F_I_SIZE(f) (OFNI_EDONI_2SFFJ(f)->i_size)
#define JFFS3_F_I_MODE(f) (OFNI_EDONI_2SFFJ(f)->i_mode)
#define JFFS3_F_I_UID(f) (OFNI_EDONI_2SFFJ(f)->i_uid)
#define JFFS3_F_I_GID(f) (OFNI_EDONI_2SFFJ(f)->i_gid)
#define JFFS3_F_I_CTIME(f) (OFNI_EDONI_2SFFJ(f)->i_ctime)
#define JFFS3_F_I_MTIME(f) (OFNI_EDONI_2SFFJ(f)->i_mtime)
#define JFFS3_F_I_ATIME(f) (OFNI_EDONI_2SFFJ(f)->i_atime)

/* FIXME: eCos doesn't hav a concept of device major/minor numbers */
#define JFFS3_F_I_RDEV_MIN(f) ((OFNI_EDONI_2SFFJ(f)->i_rdev)&0xff)
#define JFFS3_F_I_RDEV_MAJ(f) ((OFNI_EDONI_2SFFJ(f)->i_rdev)>>8)

#define get_seconds cyg_timestamp

struct _inode {
	cyg_uint32		i_ino;

	int			i_count;
	mode_t			i_mode;
	nlink_t			i_nlink; // Could we dispense with this?
	uid_t			i_uid;
	gid_t			i_gid;
	time_t			i_atime;
	time_t			i_mtime;
	time_t			i_ctime;
//	union {
		unsigned short	i_rdev; // For devices only
		struct _inode *	i_parent; // For directories only
		off_t		i_size; // For files only
//	};
	struct super_block *	i_sb;

	struct jffs3_inode_info	jffs3_i;

        struct _inode *		i_cache_prev; // We need doubly-linked?
        struct _inode *		i_cache_next;
};

#define JFFS3_SB_INFO(sb) (&(sb)->jffs3_sb)
#define OFNI_BS_2SFFJ(c)  ((struct super_block *) ( ((char *)c) - ((char *)(&((struct super_block *)NULL)->jffs3_sb)) ) )

struct super_block {
	struct jffs3_sb_info	jffs3_sb;
	struct _inode *		s_root;
        unsigned long		s_mount_count;
	cyg_io_handle_t		s_dev;
};

#define sleep_on_spinunlock(wq, sl) spin_unlock(sl)
#define EBADFD 32767

/* background.c */
#ifdef CYGOPT_FS_JFFS3_GCTHREAD
void jffs3_garbage_collect_trigger(struct jffs3_sb_info *c);
void jffs3_start_garbage_collect_thread(struct jffs3_sb_info *c);
void jffs3_stop_garbage_collect_thread(struct jffs3_sb_info *c);
#else
static inline void jffs3_garbage_collect_trigger(struct jffs3_sb_info *c)
{
	/* We don't have a GC thread in eCos (yet) */
}
#endif

/* fs-ecos.c */
struct _inode *jffs3_new_inode (struct _inode *dir_i, int mode, struct jffs3_raw_inode *ri);
struct _inode *jffs3_iget(struct super_block *sb, cyg_uint32 ino);
void jffs3_iput(struct _inode * i);
void jffs3_gc_release_inode(struct jffs3_sb_info *c, struct jffs3_inode_info *f);
struct jffs3_inode_info *jffs3_gc_fetch_inode(struct jffs3_sb_info *c, int inum, int nlink);
unsigned char *jffs3_gc_fetch_page(struct jffs3_sb_info *c, struct jffs3_inode_info *f, 
				   unsigned long offset, unsigned long *priv);
void jffs3_gc_release_page(struct jffs3_sb_info *c, unsigned char *pg, unsigned long *priv);

/* Avoid polluting eCos namespace with names not starting in jffs3_ */
#define os_to_jffs3_mode(x) jffs3_from_os_mode(x)
uint32_t jffs3_from_os_mode(uint32_t osmode);
uint32_t jffs3_to_os_mode (uint32_t jmode);


/* flashio.c */
cyg_bool jffs3_flash_read(struct jffs3_sb_info *c, cyg_uint32 read_buffer_offset,
			  const size_t size, size_t * return_size, char * write_buffer);
cyg_bool jffs3_flash_write(struct jffs3_sb_info *c, cyg_uint32 write_buffer_offset,
			   const size_t size, size_t * return_size, char * read_buffer);
int jffs3_flash_direct_writev(struct jffs3_sb_info *c, const struct iovec *vecs,
			      unsigned long count, loff_t to, size_t *retlen);
cyg_bool jffs3_flash_erase(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb);

// dir-ecos.c
struct _inode *jffs3_lookup(struct _inode *dir_i, const unsigned char *name, int namelen);
int jffs3_create(struct _inode *dir_i, const unsigned char *d_name, int mode, struct _inode **new_i);
int jffs3_mkdir (struct _inode *dir_i, const unsigned char *d_name, int mode);
int jffs3_link (struct _inode *old_d_inode, struct _inode *dir_i, const unsigned char *d_name);
int jffs3_unlink(struct _inode *dir_i, struct _inode *d_inode, const unsigned char *d_name);
int jffs3_rmdir (struct _inode *dir_i, struct _inode *d_inode, const unsigned char *d_name);
int jffs3_rename (struct _inode *old_dir_i, struct _inode *d_inode, const unsigned char *old_d_name,
		  struct _inode *new_dir_i, const unsigned char *new_d_name);

/* erase.c */
static inline void jffs3_erase_pending_trigger(struct jffs3_sb_info *c)
{ }

#ifndef CONFIG_JFFS3_FS_WRITEBUFFER
#define jffs3_can_mark_obsolete(c) (1)
#define jffs3_cleanmarker_oob(c) (0)
#define jffs3_write_nand_cleanmarker(c,jeb) (-EIO)

#define jffs3_flush_wbuf_pad(c) ({ (void)(c), 0; })
#define jffs3_flush_wbuf_gc(c, i) ({ (void)(c), (void) i, 0; })
#define jffs3_nand_read_failcnt(c,jeb) do { ; } while(0)
#define jffs3_write_nand_badblock(c,jeb,p) (0)
#define jffs3_flash_setup(c) (0)
#define jffs3_nand_flash_cleanup(c) do {} while(0)
#define jffs3_wbuf_dirty(c) (0)
#define jffs3_flash_writev(a,b,c,d,e,f) jffs3_flash_direct_writev(a,b,c,d,e)
#define jffs3_wbuf_timeout NULL
#define jffs3_wbuf_process NULL
#define jffs3_nor_ecc(c) (0)
#else
#error no nand yet
#endif

#ifndef BUG_ON
#define BUG_ON(x) do { if (unlikely(x)) BUG(); } while(0)
#endif

#define __init

#endif /* __JFFS3_OS_ECOS_H__ */

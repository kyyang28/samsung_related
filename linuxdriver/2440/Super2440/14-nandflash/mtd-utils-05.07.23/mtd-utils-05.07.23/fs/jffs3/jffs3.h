/*
 * JFFS3 -- Journalling Flash File System, Version 3.
 *
 * Copyright (C) 2001-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@infradead.org>
 *
 * For licensing information, see the file 'LICENCE' in the
 * jffs3 directory.
 *
 * $Id: jffs3.h,v 3.7 2005/02/09 14:25:14 pavlov Exp $
 */

#ifndef __LINUX_JFFS3_H__
#define __LINUX_JFFS3_H__

#include <linux/version.h>
#include <linux/rbtree.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/spinlock.h>
#include <linux/completion.h>
#include <linux/timer.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <asm/semaphore.h>
#include <linux/rwsem.h>

/*
 * You must include something which defines the C99 uintXX_t types.
 *  We don't do it from here because this file is used in too many
 * different environments.
 */

#define JFFS3_SUPER_MAGIC 0x72b6

/* Values we may expect to find in the 'magic' field */
#define JFFS3_OLD_MAGIC_BITMASK 0x1984
#define JFFS3_MAGIC_BITMASK 0x1985
#define KSAMTIB_CIGAM_2SFFJ 0x8519 /* For detecting wrong-endian fs */
#define JFFS3_EMPTY_BITMASK 0xffff
#define JFFS3_DIRTY_BITMASK 0x0000

/* Summary node MAGIC marker */
#define JFFS3_SUM_MAGIC	0x02851885

/*
 * We only allow a single char for length, and 0xFF is empty flash so
 *  we don't want it confused with a real length. Hence max 254.
 */
#define JFFS3_MAX_NAME_LEN 254

/* How small can we sensibly write nodes? */
#define JFFS3_MIN_DATA_LEN 128

#define JFFS3_COMPR_NONE	0x00
#define JFFS3_COMPR_ZERO	0x01
#define JFFS3_COMPR_RTIME	0x02
#define JFFS3_COMPR_RUBINMIPS	0x03
#define JFFS3_COMPR_COPY	0x04
#define JFFS3_COMPR_DYNRUBIN	0x05
#define JFFS3_COMPR_ZLIB	0x06
#define JFFS3_COMPR_LZO         0x07
#define JFFS3_COMPR_LZARI       0x08

/* Compatibility flags */
#define JFFS3_COMPAT_MASK	0xc000	/* What do to if an unknown nodetype is found */
#define JFFS3_NODE_ACCURATE	0x2000
/* INCOMPAT: Fail to mount the filesystem */
#define JFFS3_FEATURE_INCOMPAT	0xc000
/* ROCOMPAT: Mount read-only */
#define JFFS3_FEATURE_ROCOMPAT	0x8000
/* RWCOMPAT_COPY: Mount read/write, and copy the node when it's GC'd */
#define JFFS3_FEATURE_RWCOMPAT_COPY	0x4000
/* RWCOMPAT_DELETE: Mount read/write, and delete the node when it's GC'd */
#define JFFS3_FEATURE_RWCOMPAT_DELETE	0x0000

/* Node types */
#define JFFS3_NODETYPE_DIRENT (JFFS3_FEATURE_INCOMPAT | JFFS3_NODE_ACCURATE | 1)
#define JFFS3_NODETYPE_INODE (JFFS3_FEATURE_INCOMPAT | JFFS3_NODE_ACCURATE | 2)
#define JFFS3_NODETYPE_CLEANMARKER (JFFS3_FEATURE_RWCOMPAT_DELETE | JFFS3_NODE_ACCURATE | 3)
#define JFFS3_NODETYPE_PADDING (JFFS3_FEATURE_RWCOMPAT_DELETE | JFFS3_NODE_ACCURATE | 4)

#define JFFS3_NODETYPE_SUMMARY (JFFS3_FEATURE_RWCOMPAT_DELETE | JFFS3_NODE_ACCURATE | 6)

#define JFFS3_SB_FLAG_RO	1
#define JFFS3_SB_FLAG_MOUNTING	2

/* FIXME: these can go once we've made sure we've caught all uses without
   byteswapping */
typedef struct {
	uint32_t v32;
} __attribute__((packed))  jint32_t;

typedef struct {
	uint32_t m;
} __attribute__((packed))  jmode_t;

typedef struct {
	uint16_t v16;
} __attribute__((packed)) jint16_t;

struct jffs3_unknown_node
{
	/* All nodes start like this */
	jint16_t magic;
	jint16_t nodetype;
	jint32_t totlen;	/* So we can skip over nodes we don't grok */
	jint32_t hdr_crc;
} __attribute__((packed));

struct jffs3_raw_dirent
{
	jint16_t magic;
	jint16_t nodetype;	/* == JFFS_NODETYPE_DIRENT */
	jint32_t totlen;
	jint32_t hdr_crc;
	jint32_t pino;
	jint32_t version;
	jint32_t ino;	/* == zero for unlink */
	jint32_t mctime;
	uint8_t nsize;
	uint8_t type;
	uint8_t unused[2];
	jint32_t node_crc;
	jint32_t name_crc;
	uint8_t name[0];
} __attribute__((packed));

/*
 * The JFFS3 raw inode structure: Used for storage on physical media.
 * The uid, gid, atime, mtime and ctime members could be longer, but
 * are left like this for space efficiency. If and when people decide
 * they really need them extended, it's simple enough to add support for
 * a new type of raw node.
 */
struct jffs3_raw_inode
{
	jint16_t magic;		/* A constant magic number */
	jint16_t nodetype;	/* == JFFS_NODETYPE_INODE */
	jint32_t totlen;	/* Total length of this node (inc data, etc.) */
	jint32_t hdr_crc;
	jint32_t ino;		/* Inode number */
	jint32_t version;	/* Version number */
	jmode_t mode;		/* The file's type or mode */
	jint16_t uid;		/* The file's owner */
	jint16_t gid;		/* The file's group */
	jint32_t isize;		/* Total resultant size of this inode (used for truncations) */
	jint32_t atime;		/* Last access time */
	jint32_t mtime;		/* Last modification time */
	jint32_t ctime;		/* Change time */
	jint32_t offset;	/* Where to begin to write */
	jint32_t csize;		/* (Compressed) data size */
	jint32_t dsize;		/* Size of the node's data (after decompression) */
	uint8_t compr;		/* Compression algorithm used */
	uint8_t usercompr;	/* Compression algorithm requested by the user */
	jint16_t flags;		/* See JFFS3_INO_FLAG_* */
	jint32_t data_crc;	/* CRC for the (compressed) data */
	jint32_t node_crc;	/* CRC for the raw inode (excluding data) */
	uint8_t data[0];
} __attribute__((packed));

struct jffs3_summary_node{
	jint16_t magic;
	jint16_t nodetype; 	/* = JFFS3_NODETYPE_INODE_SUM */
	jint32_t totlen;
	jint32_t hdr_crc;
	jint16_t sum_num;	/* number of sum entries*/
	jint32_t cln_mkr;	/* clean marker size, 0 = no cleanmarker */
	jint32_t padded;        /* sum of the size of padding nodes */
	jint32_t sum_crc;	/* summary information crc */
	jint32_t node_crc; 	/* node crc */
	jint32_t sum[0]; 	/* inode summary info */
} __attribute__((packed));

union jffs3_node_union {
	struct jffs3_raw_inode i;
	struct jffs3_raw_dirent d;
	struct jffs3_unknown_node u;
	struct jffs3_summary_node s;
};


struct jffs3_inode_info {
	/* We need an internal semaphore similar to inode->i_sem.
	   Unfortunately, we can't used the existing one, because
	   either the GC would deadlock, or we'd have to release it
	   before letting GC proceed. Or we'd have to put ugliness
	   into the GC code so it didn't attempt to obtain the i_sem
	   for the inode(s) which are already locked */
	struct semaphore sem;

	/* The highest (datanode) version number used for this ino */
	uint32_t highest_version;

	/* List of data fragments which make up the file */
	struct rb_root fragtree;

	/* There may be one datanode which isn't referenced by any of the
	   above fragments, if it contains a metadata update but no actual
	   data - or if this is a directory inode.
	   This also holds the _only_ dnode for symlinks/device nodes, etc. */
	struct jffs3_full_dnode *metadata;

	/* Directory entries */
	struct jffs3_full_dirent *dents;

	/* Some stuff we just have to keep in-core at all times, for each inode */
	struct jffs3_inode_cache *inocache;

	uint16_t flags;
	uint8_t usercompr;

	struct inode vfs_inode;
};

struct jffs3_inodirty;

/*
 * A struct for the overall file system control. Pointers to
 * jffs3_sb_info structs are named `c' in the source code.
 */
struct jffs3_sb_info {
	struct mtd_info *mtd;

	uint32_t highest_ino;
	uint32_t checked_ino;

	unsigned int flags;

	struct task_struct *gc_task;		/* GC task struct */
	struct semaphore gc_thread_start;	/* GC thread start mutex */
	struct completion gc_thread_exit;	/* GC thread exit completion port */

	struct semaphore alloc_sem;	/* Used to protect all the following
					   fields, and also to protect against
					   out-of-order writing of nodes. And GC. */
	uint32_t cleanmarker_size;	/* Size of an _inline_ CLEANMARKER
					 (i.e. zero for OOB CLEANMARKER */

	uint32_t flash_size;
	uint32_t used_size;
	uint32_t dirty_size;
	uint32_t wasted_size;
	uint32_t free_size;
	uint32_t erasing_size;
	uint32_t bad_size;
	uint32_t sector_size;
	uint32_t unchecked_size;

	uint32_t nr_free_blocks;
	uint32_t nr_erasing_blocks;

	/* Number of free blocks there must be before we... */
	uint8_t resv_blocks_write;	/* ... allow a normal filesystem write */
	uint8_t resv_blocks_deletion;	/* ... allow a normal filesystem deletion */
	uint8_t resv_blocks_gctrigger;	/* ... wake up the GC thread */
	uint8_t resv_blocks_gcbad;	/* ... pick a block from the bad_list to GC */
	uint8_t resv_blocks_gcmerge;	/* ... merge pages when garbage collecting */

	uint32_t nospc_dirty_size;

	uint32_t nr_blocks;
	struct jffs3_eraseblock *blocks;	/* The whole array of blocks. Used for getting blocks
						 * from the offset (blocks[ofs / sector_size]) */
	struct jffs3_eraseblock *nextblock;	/* The block we're currently filling */

	struct jffs3_eraseblock *gcblock;	/* The block we're currently garbage-collecting */

	struct list_head clean_list;		/* Blocks 100% full of clean data */
	struct list_head very_dirty_list;	/* Blocks with lots of dirty space */
	struct list_head dirty_list;		/* Blocks with some dirty space */
	struct list_head erasable_list;		/* Blocks which are completely dirty, and need erasing */
	struct list_head erasable_pending_wbuf_list;	/* Blocks which need erasing but only after the
							   current wbuf is flushed */
	struct list_head erasing_list;		/* Blocks which are currently erasing */
	struct list_head erase_pending_list;	/* Blocks which need erasing now */
	struct list_head erase_complete_list;	/* Blocks which are erased and need the clean marker written to them */
	struct list_head free_list;		/* Blocks which are free and ready to be used */
	struct list_head bad_list;		/* Bad blocks. */
	struct list_head bad_used_list;		/* Bad blocks with valid data in. */

	spinlock_t erase_completion_lock;	/* Protect free_list and erasing_list
						   against erase completion handler */
	wait_queue_head_t erase_wait;		/* For waiting for erases to complete */

	wait_queue_head_t inocache_wq;
	struct jffs3_inode_cache **inocache_list;
	spinlock_t inocache_lock;

	/* Semaphore to allow jffs3_garbage_collect_deletion_dirent to
	   drop the erase_completion_lock while it's holding a pointer
	   to an obsoleted node. I don't like this. Alternatives welcomed. */
	struct semaphore erase_free_sem;

#ifdef CONFIG_JFFS3_FS_WRITEBUFFER
	/* Write-behind buffer for NAND flash */
	unsigned char *wbuf;
	uint32_t wbuf_ofs;
	uint32_t wbuf_len;
	uint32_t wbuf_pagesize;
	struct jffs3_inodirty *wbuf_inodes;
	struct rw_semaphore wbuf_sem;	/* Protects the write buffer */

	/* Information about out-of-band area usage... */
	struct nand_oobinfo *oobinfo;
	uint32_t badblock_pos;
	uint32_t fsdata_pos;
	uint32_t fsdata_len;
#endif
#ifdef CONFIG_JFFS3_SUMMARY
    jint32_t *summary_buf;
#endif

	/* OS-private pointer for getting back to master superblock info */
	void *os_priv;
};

#endif /* __LINUX_JFFS3_H__ */

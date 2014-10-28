/*
 * JFFS3 -- Journalling Flash File System, Version 3.
 *
 * Copyright (C) 2001-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@infradead.org>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * JFFS2 Id: nodelist.h,v 1.126 2004/11/19 15:06:29 dedekind  Exp
 * $Id: nodelist.h,v 3.12 2005/02/09 14:25:14 pavlov Exp $
 *
 */

#ifndef __JFFS3_NODELIST_H__
#define __JFFS3_NODELIST_H__

#include <linux/config.h>
#include <linux/fs.h>
#include <linux/types.h>
#include "jffs3.h"

#ifdef __ECOS
#include "os-ecos.h"
#else
#include <linux/mtd/compatmac.h> /* For min/max in older kernels */
#include "os-linux.h"
#endif

/*
 * TODO: Temporary. When the __totlen field removing is completed, all #ifdefs
 * with TMP_TOTLEN should go.
 * Note: The summary stuff is currently broken with regards to __totlen.
 */
#define TMP_TOTLEN

#define JFFS3_NATIVE_ENDIAN

/* Note we handle mode bits conversion from JFFS3 (i.e. Linux) to/from
   whatever OS we're actually running on here too. */

#if defined(JFFS3_NATIVE_ENDIAN)
#define cpu_to_je16(x) ((jint16_t){x})
#define cpu_to_je32(x) ((jint32_t){x})
#define cpu_to_jemode(x) ((jmode_t){os_to_jffs3_mode(x)})

#define je16_to_cpu(x) ((x).v16)
#define je32_to_cpu(x) ((x).v32)
#define jemode_to_cpu(x) (jffs3_to_os_mode((x).m))
#elif defined(JFFS3_BIG_ENDIAN)
#define cpu_to_je16(x) ((jint16_t){cpu_to_be16(x)})
#define cpu_to_je32(x) ((jint32_t){cpu_to_be32(x)})
#define cpu_to_jemode(x) ((jmode_t){cpu_to_be32(os_to_jffs3_mode(x))})

#define je16_to_cpu(x) (be16_to_cpu(x.v16))
#define je32_to_cpu(x) (be32_to_cpu(x.v32))
#define jemode_to_cpu(x) (be32_to_cpu(jffs3_to_os_mode((x).m)))
#elif defined(JFFS3_LITTLE_ENDIAN)
#define cpu_to_je16(x) ((jint16_t){cpu_to_le16(x)})
#define cpu_to_je32(x) ((jint32_t){cpu_to_le32(x)})
#define cpu_to_jemode(x) ((jmode_t){cpu_to_le32(os_to_jffs3_mode(x))})

#define je16_to_cpu(x) (le16_to_cpu(x.v16))
#define je32_to_cpu(x) (le32_to_cpu(x.v32))
#define jemode_to_cpu(x) (le32_to_cpu(jffs3_to_os_mode((x).m)))
#else
#error wibble
#endif

/*
  This is all we need to keep in-core for each raw node during normal
  operation. As and when we do read_inode on a particular inode, we can
  scan the nodes which are listed for it and build up a proper map of
  which nodes are currently valid. JFFSv1 always used to keep that whole
  map in core for each inode.
*/
struct jffs3_raw_node_ref
{
	struct jffs3_raw_node_ref *next_in_ino; /* Points to the next raw_node_ref
		for this inode. If this is the last, it points to the inode_cache
		for this inode instead. The inode_cache will have NULL in the first
		word so you know when you've got there :) */
	struct jffs3_raw_node_ref *next_phys;
	uint32_t flash_offset;
#ifdef TMP_TOTLEN
	uint32_t __totlen; /* This may die; use ref_totlen(c, jeb, ) below */
#endif
};

        /* flash_offset & 3 always has to be zero, because nodes are
	   always aligned at 4 bytes. So we have a couple of extra bits
	   to play with, which indicate the node's status; see below: */
#define REF_UNCHECKED	0	/* We haven't yet checked the CRC or built its inode */
#define REF_OBSOLETE	1	/* Obsolete, can be completely ignored */
#define REF_PRISTINE	2	/* Completely clean. GC without looking */
#define REF_NORMAL	3	/* Possibly overlapped. Read the page and write again on GC */
#define ref_flags(ref)		((ref)->flash_offset & 3)
#define ref_offset(ref)		((ref)->flash_offset & ~3)
#define ref_obsolete(ref)	(((ref)->flash_offset & 3) == REF_OBSOLETE)
#define mark_ref_normal(ref)    do { (ref)->flash_offset = ref_offset(ref) | REF_NORMAL; } while(0)

/* For each inode in the filesystem, we need to keep a record of
   nlink, because it would be a PITA to scan the whole directory tree
   at read_inode() time to calculate it, and to keep sufficient information
   in the raw_node_ref (basically both parent and child inode number for
   dirent nodes) would take more space than this does. We also keep
   a pointer to the first physical node which is part of this inode, too.
*/
struct jffs3_inode_cache {
	struct jffs3_full_dirent *scan_dents; /* Used during scan to hold
		temporary lists of dirents, and later must be set to
		NULL to mark the end of the raw_node_ref->next_in_ino
		chain. */
	struct jffs3_inode_cache *next;
	struct jffs3_raw_node_ref *nodes;
	uint32_t ino;
	int nlink;
	int state;
};

/* Inode states for 'state' above. We need the 'GC' state to prevent
   someone from doing a read_inode() while we're moving a 'REF_PRISTINE'
   node without going through all the iget() nonsense */
#define INO_STATE_UNCHECKED	0	/* CRC checks not yet done */
#define INO_STATE_CHECKING	1	/* CRC checks in progress */
#define INO_STATE_PRESENT	2	/* In core */
#define INO_STATE_CHECKEDABSENT	3	/* Checked, cleared again */
#define INO_STATE_GC		4	/* GCing a 'pristine' node */
#define INO_STATE_READING	5	/* In read_inode() */

#define INOCACHE_HASHSIZE 128

/*
  Larger representation of a raw node, kept in-core only when the
  struct inode for this particular ino is instantiated.
*/

struct jffs3_full_dnode
{
	struct jffs3_raw_node_ref *raw;
	uint32_t ofs; /* The offset to which the data of this node belongs */
	uint32_t size;
	uint32_t frags; /* Number of fragments which currently refer
			to this node. When this reaches zero,
			the node is obsolete.  */
};

/*
   Even larger representation of a raw node, kept in-core only while
   we're actually building up the original map of which nodes go where,
   in read_inode()
*/
struct jffs3_tmp_dnode_info
{
	struct jffs3_tmp_dnode_info *next;
	struct jffs3_full_dnode *fn;
	uint32_t version;
};

struct jffs3_full_dirent
{
	struct jffs3_raw_node_ref *raw;
	struct jffs3_full_dirent *next;
	uint32_t version;
	uint32_t ino; /* == zero for unlink */
	unsigned int nhash;
	unsigned char type;
	unsigned char name[0];
};

/*
  Fragments - used to build a map of which raw node to obtain
  data from for each part of the ino
*/
struct jffs3_node_frag
{
	struct rb_node rb;
	struct jffs3_full_dnode *node; /* NULL for holes */
	uint32_t size;
	uint32_t ofs; /* The offset to which this fragment belongs */
};

struct jffs3_eraseblock
{
	struct list_head list;
	int bad_count;
	uint32_t offset;		/* of this block in the MTD */

#ifdef CONFIG_JFFS3_SUMMARY
	struct jffs3_sum_info *sum_collected;
#endif

	uint32_t unchecked_size;
	uint32_t used_size;
	uint32_t dirty_size;
	uint32_t wasted_size;
	uint32_t free_size;	/* Note that sector_size - free_size
				   is the address of the first free space */
	struct jffs3_raw_node_ref *first_node;
	struct jffs3_raw_node_ref *last_node;

	struct jffs3_raw_node_ref *gc_node;	/* Next node to be garbage collected */
};

#include "debug.h"

/* Calculate totlen from surrounding nodes or eraseblock */
static inline uint32_t
ref_totlen(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb,
			struct jffs3_raw_node_ref *ref)
{
	uint32_t ref_end;
	uint32_t ref_offs = ref_offset(ref);

	if (!jeb)
		jeb = &c->blocks[ref->flash_offset / c->sector_size];

	if (ref->next_phys)
		ref_end = ref_offset(ref->next_phys);
	else {
#ifdef TMP_TOTLEN
		if (ref != jeb->last_node) {
			ERROR_MSG("ref %p, offset %#x, __totlen %u\n", ref, ref_offs, ref->__totlen);
			ERROR_MSG("ref->next_phys is NULL, but this is not the last node for this block\n");
			if (DEBUG1)
				jffs3_dbg_dump_node_refs(c, jeb);
			return ref->__totlen;
		}
#else
		if (SANITY)
			BUG_ON(ref != jeb->last_node);
#endif
		/* Last node in block. Use free_space */
		ref_end = jeb->offset + c->sector_size - jeb->free_size;
	}

	if (PARANOIA && jeb && jeb != &c->blocks[ref->flash_offset / c->sector_size]) {
		ERROR_MSG("ref_totlen called with wrong block - %#08x instead of %#08x; ref 0x%08x\n",
		       jeb->offset, c->blocks[ref->flash_offset / c->sector_size].offset, ref_offs);
		BUG();
	}

#ifdef TMP_TOTLEN
	if (ref_end - ref_offs != ref->__totlen
		/* Last obsolete node may have wrong length */
		&& ref != jeb->last_node) {
		ERROR_MSG("Ref (%p) totlen %#x (%#08x-%#08x) miscalculated as "
			"%#x (%#08x-%#08x) instead of %#x\n",
			ref, ref->__totlen, ref_offs, ref_offs + ref->__totlen,
			ref_end - ref_offs, ref_offs, ref_end, ref->__totlen);
		if (DEBUG1)
			jffs3_dbg_dump_node_refs(c, jeb);
	}
	return ref->__totlen;
#endif
	return ref_end - ref_offs;
}

#define ALLOC_NORMAL	0	/* Normal allocation */
#define ALLOC_DELETION	1	/* Deletion node. Best to allow it */
#define ALLOC_GC	2	/* Space requested for GC. Give it or die */
#define ALLOC_NORETRY	3	/* For jffs3_write_dnode: On failure, return -EAGAIN instead of retrying */

/* How much dirty space before it goes on the very_dirty_list */
#define VERYDIRTY(c, size) ((size) >= ((c)->sector_size / 2))

/* check if dirty space is more than 255 Byte */
#define ISDIRTY(size) ((size) >  sizeof (struct jffs3_raw_inode) + JFFS3_MIN_DATA_LEN)

#define PAD(x) (((x)+3)&~3)

static inline struct jffs3_inode_cache *jffs3_raw_ref_to_ic(struct jffs3_raw_node_ref *raw)
{
	while(raw->next_in_ino) {
		raw = raw->next_in_ino;
	}

	return ((struct jffs3_inode_cache *)raw);
}

static inline struct jffs3_node_frag *frag_first(struct rb_root *root)
{
	struct rb_node *node = root->rb_node;

	if (!node)
		return NULL;
	while(node->rb_left)
		node = node->rb_left;
	return rb_entry(node, struct jffs3_node_frag, rb);
}
#define rb_parent(rb) ((rb)->rb_parent)
#define frag_next(frag) rb_entry(rb_next(&(frag)->rb), struct jffs3_node_frag, rb)
#define frag_prev(frag) rb_entry(rb_prev(&(frag)->rb), struct jffs3_node_frag, rb)
#define frag_parent(frag) rb_entry(rb_parent(&(frag)->rb), struct jffs3_node_frag, rb)
#define frag_left(frag) rb_entry((frag)->rb.rb_left, struct jffs3_node_frag, rb)
#define frag_right(frag) rb_entry((frag)->rb.rb_right, struct jffs3_node_frag, rb)
#define frag_erase(frag, list) rb_erase(&frag->rb, list);

/* nodelist.c */
void jffs3_add_fd_to_list(struct jffs3_sb_info *c, struct jffs3_full_dirent *new, struct jffs3_full_dirent **list);
int jffs3_get_inode_nodes(struct jffs3_sb_info *c, struct jffs3_inode_info *f,
			  struct jffs3_tmp_dnode_info **tnp, struct jffs3_full_dirent **fdp,
			  uint32_t *highest_version, uint32_t *latest_mctime,
			  uint32_t *mctime_ver);
void jffs3_set_inocache_state(struct jffs3_sb_info *c, struct jffs3_inode_cache *ic, int state);
struct jffs3_inode_cache *jffs3_get_ino_cache(struct jffs3_sb_info *c, uint32_t ino);
void jffs3_add_ino_cache (struct jffs3_sb_info *c, struct jffs3_inode_cache *new);
void jffs3_del_ino_cache(struct jffs3_sb_info *c, struct jffs3_inode_cache *old);
void jffs3_free_ino_caches(struct jffs3_sb_info *c);
void jffs3_free_raw_node_refs(struct jffs3_sb_info *c);
struct jffs3_node_frag *jffs3_lookup_node_frag(struct rb_root *fragtree, uint32_t offset);
void jffs3_kill_fragtree(struct rb_root *root, struct jffs3_sb_info *c_delete);
void jffs3_fragtree_insert(struct jffs3_node_frag *newfrag, struct jffs3_node_frag *base);
struct rb_node *rb_next(struct rb_node *);
struct rb_node *rb_prev(struct rb_node *);
void rb_replace_node(struct rb_node *victim, struct rb_node *new, struct rb_root *root);

/* nodemgmt.c */
int jffs3_thread_should_wake(struct jffs3_sb_info *c);
int jffs3_reserve_space(struct jffs3_sb_info *c, uint32_t minsize, uint32_t *ofs, uint32_t *len, int prio, uint32_t sumsize);
int jffs3_reserve_space_gc(struct jffs3_sb_info *c, uint32_t minsize, uint32_t *ofs, uint32_t *len, uint32_t sumsize);
int jffs3_add_physical_node_ref(struct jffs3_sb_info *c, struct jffs3_raw_node_ref *new, uint32_t len);
void jffs3_complete_reservation(struct jffs3_sb_info *c);
void jffs3_mark_node_obsolete(struct jffs3_sb_info *c, struct jffs3_raw_node_ref *raw);

/* write.c */
int jffs3_do_new_inode(struct jffs3_sb_info *c, struct jffs3_inode_info *f, uint32_t mode, struct jffs3_raw_inode *ri);

struct jffs3_full_dnode *jffs3_write_dnode(struct jffs3_sb_info *c, struct jffs3_inode_info *f, struct jffs3_raw_inode *ri, const unsigned char *data, uint32_t datalen, uint32_t flash_ofs, int alloc_mode);
struct jffs3_full_dirent *jffs3_write_dirent(struct jffs3_sb_info *c, struct jffs3_inode_info *f, struct jffs3_raw_dirent *rd, const unsigned char *name, uint32_t namelen, uint32_t flash_ofs, int alloc_mode);
int jffs3_write_inode_range(struct jffs3_sb_info *c, struct jffs3_inode_info *f,
			    struct jffs3_raw_inode *ri, unsigned char *buf,
			    uint32_t offset, uint32_t writelen, uint32_t *retlen);
int jffs3_do_create(struct jffs3_sb_info *c, struct jffs3_inode_info *dir_f, struct jffs3_inode_info *f, struct jffs3_raw_inode *ri, const char *name, int namelen);
int jffs3_do_unlink(struct jffs3_sb_info *c, struct jffs3_inode_info *dir_f, const char *name, int namelen, struct jffs3_inode_info *dead_f);
int jffs3_do_link (struct jffs3_sb_info *c, struct jffs3_inode_info *dir_f, uint32_t ino, uint8_t type, const char *name, int namelen);


/* readinode.c */
void jffs3_truncate_fraglist (struct jffs3_sb_info *c, struct rb_root *list, uint32_t size);
int jffs3_add_full_dnode_to_inode(struct jffs3_sb_info *c, struct jffs3_inode_info *f, struct jffs3_full_dnode *fn);
int jffs3_do_read_inode(struct jffs3_sb_info *c, struct jffs3_inode_info *f,
			uint32_t ino, struct jffs3_raw_inode *latest_node);
int jffs3_do_crccheck_inode(struct jffs3_sb_info *c, struct jffs3_inode_cache *ic);
void jffs3_do_clear_inode(struct jffs3_sb_info *c, struct jffs3_inode_info *f);

/* malloc.c */
int jffs3_create_slab_caches(void);
void jffs3_destroy_slab_caches(void);

struct jffs3_full_dirent *jffs3_alloc_full_dirent(int namesize);
void jffs3_free_full_dirent(struct jffs3_full_dirent *);
struct jffs3_full_dnode *jffs3_alloc_full_dnode(void);
void jffs3_free_full_dnode(struct jffs3_full_dnode *);
struct jffs3_raw_dirent *jffs3_alloc_raw_dirent(void);
void jffs3_free_raw_dirent(struct jffs3_raw_dirent *);
struct jffs3_raw_inode *jffs3_alloc_raw_inode(void);
void jffs3_free_raw_inode(struct jffs3_raw_inode *);
struct jffs3_tmp_dnode_info *jffs3_alloc_tmp_dnode_info(void);
void jffs3_free_tmp_dnode_info(struct jffs3_tmp_dnode_info *);
struct jffs3_raw_node_ref *jffs3_alloc_raw_node_ref(void);
void jffs3_free_raw_node_ref(struct jffs3_raw_node_ref *);
struct jffs3_node_frag *jffs3_alloc_node_frag(void);
void jffs3_free_node_frag(struct jffs3_node_frag *);
struct jffs3_inode_cache *jffs3_alloc_inode_cache(void);
void jffs3_free_inode_cache(struct jffs3_inode_cache *);

/* gc.c */
int jffs3_garbage_collect_pass(struct jffs3_sb_info *c);

/* read.c */
int jffs3_read_dnode(struct jffs3_sb_info *c, struct jffs3_inode_info *f,
		     struct jffs3_full_dnode *fd, unsigned char *buf,
		     int ofs, int len);
int jffs3_read_inode_range(struct jffs3_sb_info *c, struct jffs3_inode_info *f,
			   unsigned char *buf, uint32_t offset, uint32_t len);
char *jffs3_getlink(struct jffs3_sb_info *c, struct jffs3_inode_info *f);

/* scan.c */
int jffs3_scan_medium(struct jffs3_sb_info *c);
void jffs3_rotate_lists(struct jffs3_sb_info *c);
int jffs3_fill_scan_buf (struct jffs3_sb_info *c, unsigned char *buf,
			uint32_t ofs, uint32_t len);
struct jffs3_inode_cache *jffs3_scan_make_ino_cache(struct jffs3_sb_info *c, uint32_t ino);

/* build.c */
int jffs3_do_mount_fs(struct jffs3_sb_info *c);

/* erase.c */
void jffs3_erase_block(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb);
void jffs3_erase_pending_blocks(struct jffs3_sb_info *c, int count);

#ifdef CONFIG_JFFS3_FS_WRITEBUFFER
/* wbuf.c */
int jffs3_flush_wbuf_gc(struct jffs3_sb_info *c, uint32_t ino);
int jffs3_flush_wbuf_pad(struct jffs3_sb_info *c);
int jffs3_check_nand_cleanmarker(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb);
int jffs3_write_nand_cleanmarker(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb);
#endif

#endif /* __JFFS3_NODELIST_H__ */


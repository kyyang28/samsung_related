/*
 * JFFS3 -- Journalling Flash File System, Version 2.
 *
 * Copyright (C) 2001-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@infradead.org>
 *
 * For licensing information, see the file 'LICENCE' in the 
 * jffs3 directory.
 *
 * $Id: jffs3.h,v 1.2 2005/01/21 11:52:23 havasi Exp $
 */

#ifndef __LINUX_JFFS3_H__
#define __LINUX_JFFS3_H__
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
	jint16_t nodetype; 	/* = JFFS3_NODETYPE_SUMMARY */
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

#include <endian.h>
#include <byteswap.h>

#undef cpu_to_je16
#undef cpu_to_je32
#undef cpu_to_jemode
#undef je16_to_cpu
#undef je32_to_cpu
#undef jemode_to_cpu

extern int target_endian;

#define t16(x) ({ uint16_t __b = (x); (target_endian==__BYTE_ORDER)?__b:bswap_16(__b); })
#define t32(x) ({ uint32_t __b = (x); (target_endian==__BYTE_ORDER)?__b:bswap_32(__b); })

#define cpu_to_je16(x) ((jint16_t){t16(x)})
#define cpu_to_je32(x) ((jint32_t){t32(x)})
#define cpu_to_jemode(x) ((jmode_t){t32(x)})

#define je16_to_cpu(x) (t16((x).v16))
#define je32_to_cpu(x) (t32((x).v32))
#define jemode_to_cpu(x) (t32((x).m))

#endif /* __LINUX_JFFS3_H__ */

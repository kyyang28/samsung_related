/*
 * JFFS3 -- Journalling Flash File System, Version 3.
 *
 * Copyright (C) 2004  Ferenc Havasi <havasi@inf.u-szeged.hu>,
 *                     Zoltan Sogor <weth@inf.u-szeged.hu>,
 *                     Patrik Kluba <pajko@halom.u-szeged.hu>,
 *                     University of Szeged, Hungary
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * $Id: summary.h,v 3.4 2005/04/14 09:12:09 havasi Exp $
 *
 */

#ifndef __JFFS3_SUMMARY_H__
#define __JFFS3_SUMMARY_H__

#include <linux/uio.h>
#include "jffs3.h"
#include "scan.h"

struct jffs3_sum_unknown_flash
{
	jint16_t nodetype;	/* node type	*/
};

struct jffs3_sum_inode_flash
{
	jint16_t nodetype;	/* node type	*/
	jint32_t inode;		/* inode number */
	jint32_t version;	/* inode version */
	jint32_t offset;	/* offset on jeb */
	jint32_t totlen; 	/* record length */
} __attribute__((packed));

struct jffs3_sum_dirent_flash
{
	jint16_t nodetype;	/* == JFFS3_NODETYPE_DIRENT */
	jint32_t totlen;	/* record length */
	jint32_t offset;	/* ofset on jeb */
	jint32_t pino;		/* parent inode */
	jint32_t version;	/* dirent version */
	jint32_t ino; 		/* == zero for unlink */
	uint8_t nsize;		/* dirent name size */
	uint8_t type;		/* dirent type */
	uint8_t name[0];	/* dirent name */
} __attribute__((packed));

union jffs3_sum_flash{
	struct jffs3_sum_unknown_flash u;
	struct jffs3_sum_inode_flash i;
	struct jffs3_sum_dirent_flash d;
};

/* list version of jffs3_sum_*flash for kernel and sumtool */
struct jffs3_sum_unknown_mem
{
	union jffs3_sum_mem *next;
	jint16_t nodetype;	/* node type	*/

};

struct jffs3_sum_inode_mem
{
	union jffs3_sum_mem *next;
	jint16_t nodetype;	/* node type	*/
	jint32_t inode;		/* inode number */
	jint32_t version;	/* inode version */
	jint32_t offset;	/* offset on jeb */
	jint32_t totlen; 	/* record length */
} __attribute__((packed));

struct jffs3_sum_dirent_mem
{
	union jffs3_sum_mem *next;
	jint16_t nodetype;	/* == JFFS3_NODETYPE_DIRENT */
	jint32_t totlen;	/* record length */
	jint32_t offset;	/* ofset on jeb */
	jint32_t pino;		/* parent inode */
	jint32_t version;	/* dirent version */
	jint32_t ino; 		/* == zero for unlink */
	uint8_t nsize;		/* dirent name size */
	uint8_t type;		/* dirent type */
	uint8_t name[0];	/* dirent name */
} __attribute__((packed));

union jffs3_sum_mem
{
	struct jffs3_sum_unknown_mem u;
	struct jffs3_sum_inode_mem i;
	struct jffs3_sum_dirent_mem d;
};

struct jffs3_sum_info
{
	uint32_t sum_size;
	uint32_t sum_num;
    uint32_t sum_padded;
	union jffs3_sum_mem *sum_list;
};

struct jffs3_sum_marker
{
	jint32_t offset;
	jint32_t erase_size;
	jint32_t magic;
};

#define JFFS3_SUMMARY_FRAME_SIZE (sizeof(struct jffs3_summary_node) + sizeof(struct jffs3_sum_marker))

#if !(defined(SUM_TOOL) || defined(JFFS2DUMP))

int jffs3_sum_init(struct jffs3_sb_info *c);
void jffs3_sum_exit(struct jffs3_sb_info *c);

int jffs3_sum_care_sum_collected(struct jffs3_eraseblock *jeb);

void jffs3_sum_clean_collected(struct jffs3_eraseblock *jeb);
void jffs3_sum_clean_all_info(struct jffs3_sb_info *c); /* clean up all summary information in all jeb (umount) */

int jffs3_sum_add_padding_mem(struct jffs3_eraseblock *jeb, uint32_t size);
int jffs3_sum_add_inode_mem(struct jffs3_eraseblock *jeb, struct jffs3_raw_inode *ri, uint32_t ofs);
int jffs3_sum_add_dirent_mem(struct jffs3_eraseblock *jeb, struct jffs3_raw_dirent *rd, uint32_t ofs);
int jffs3_sum_add_kvec(struct jffs3_sb_info *c, const struct kvec *invecs, unsigned long count,  uint32_t to);

int jffs3_sum_scan_sumnode(struct jffs3_sb_info *c, struct jffs3_eraseblock *jeb, uint32_t ofs, uint32_t *pseudo_random);
int jffs3_sum_write_sumnode(struct jffs3_sb_info *c);

#endif

#endif /* __JFFS3_SUMMARY_H__ */

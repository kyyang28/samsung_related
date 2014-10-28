/*
 * JFFS3 -- Journalling Flash File System, Version 3.
 *
 * Copyright (C) 2001-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@infradead.org>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * JFFS2 Id: malloc.c,v 1.28 2004/11/16 20:36:11 dwmw2  Exp
 * $Id: malloc.c,v 3.6 2005/01/05 16:19:00 dedekind Exp $
 *
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include "jffs3.h"
#include "nodelist.h"

#define JFFS3_SLAB_POISON 0

/* These are initialised to NULL in the kernel startup code.
   If you're porting to other operating systems, beware */
static kmem_cache_t *full_dnode_slab;
static kmem_cache_t *raw_dirent_slab;
static kmem_cache_t *raw_inode_slab;
static kmem_cache_t *tmp_dnode_info_slab;
static kmem_cache_t *raw_node_ref_slab;
static kmem_cache_t *node_frag_slab;
static kmem_cache_t *inode_cache_slab;

int __init jffs3_create_slab_caches(void)
{
	full_dnode_slab = kmem_cache_create("jffs3_full_dnode",
					    sizeof(struct jffs3_full_dnode),
					    0, JFFS3_SLAB_POISON, NULL, NULL);
	if (!full_dnode_slab)
		goto err;

	raw_dirent_slab = kmem_cache_create("jffs3_raw_dirent",
					    sizeof(struct jffs3_raw_dirent),
					    0, JFFS3_SLAB_POISON, NULL, NULL);
	if (!raw_dirent_slab)
		goto err;

	raw_inode_slab = kmem_cache_create("jffs3_raw_inode",
					   sizeof(struct jffs3_raw_inode),
					   0, JFFS3_SLAB_POISON, NULL, NULL);
	if (!raw_inode_slab)
		goto err;

	tmp_dnode_info_slab = kmem_cache_create("jffs3_tmp_dnode",
						sizeof(struct jffs3_tmp_dnode_info),
						0, JFFS3_SLAB_POISON, NULL, NULL);
	if (!tmp_dnode_info_slab)
		goto err;

	raw_node_ref_slab = kmem_cache_create("jffs3_raw_node_ref",
					      sizeof(struct jffs3_raw_node_ref),
					      0, JFFS3_SLAB_POISON, NULL, NULL);
	if (!raw_node_ref_slab)
		goto err;

	node_frag_slab = kmem_cache_create("jffs3_node_frag",
					   sizeof(struct jffs3_node_frag),
					   0, JFFS3_SLAB_POISON, NULL, NULL);
	if (!node_frag_slab)
		goto err;

	inode_cache_slab = kmem_cache_create("jffs3_inode_cache",
					     sizeof(struct jffs3_inode_cache),
					     0, JFFS3_SLAB_POISON, NULL, NULL);
	if (inode_cache_slab)
		return 0;
 err:
	jffs3_destroy_slab_caches();
	return -ENOMEM;
}

void jffs3_destroy_slab_caches(void)
{
	if(full_dnode_slab)
		kmem_cache_destroy(full_dnode_slab);
	if(raw_dirent_slab)
		kmem_cache_destroy(raw_dirent_slab);
	if(raw_inode_slab)
		kmem_cache_destroy(raw_inode_slab);
	if(tmp_dnode_info_slab)
		kmem_cache_destroy(tmp_dnode_info_slab);
	if(raw_node_ref_slab)
		kmem_cache_destroy(raw_node_ref_slab);
	if(node_frag_slab)
		kmem_cache_destroy(node_frag_slab);
	if(inode_cache_slab)
		kmem_cache_destroy(inode_cache_slab);
}

struct jffs3_full_dirent *jffs3_alloc_full_dirent(int namesize)
{
	return kmalloc(sizeof(struct jffs3_full_dirent) + namesize, GFP_KERNEL);
}

void jffs3_free_full_dirent(struct jffs3_full_dirent *x)
{
	kfree(x);
}

struct jffs3_full_dnode *jffs3_alloc_full_dnode(void)
{
	struct jffs3_full_dnode *ret = kmem_cache_alloc(full_dnode_slab, GFP_KERNEL);
	DBG_SALLOC(1, "alloc_full_dnode at %p\n", ret);
	return ret;
}

void jffs3_free_full_dnode(struct jffs3_full_dnode *x)
{
	DBG_SALLOC(1, "free full_dnode at %p\n", x);
	kmem_cache_free(full_dnode_slab, x);
}

struct jffs3_raw_dirent *jffs3_alloc_raw_dirent(void)
{
	struct jffs3_raw_dirent *ret = kmem_cache_alloc(raw_dirent_slab, GFP_KERNEL);
	DBG_SALLOC(1, "alloc_raw_dirent %p\n", ret);
	return ret;
}

void jffs3_free_raw_dirent(struct jffs3_raw_dirent *x)
{
	DBG_SALLOC(1, "free_raw_dirent at %p\n", x);
	kmem_cache_free(raw_dirent_slab, x);
}

struct jffs3_raw_inode *jffs3_alloc_raw_inode(void)
{
	struct jffs3_raw_inode *ret = kmem_cache_alloc(raw_inode_slab, GFP_KERNEL);
	DBG_SALLOC(1, "alloc_raw_inode at %p\n", ret);
	return ret;
}

void jffs3_free_raw_inode(struct jffs3_raw_inode *x)
{
	DBG_SALLOC(1, "free_raw_inode at %p\n", x);
	kmem_cache_free(raw_inode_slab, x);
}

struct jffs3_tmp_dnode_info *jffs3_alloc_tmp_dnode_info(void)
{
	struct jffs3_tmp_dnode_info *ret = kmem_cache_alloc(tmp_dnode_info_slab, GFP_KERNEL);
	DBG_SALLOC(1, "alloc_tmp_dnode_info at %p\n", ret);
	return ret;
}

void jffs3_free_tmp_dnode_info(struct jffs3_tmp_dnode_info *x)
{
	DBG_SALLOC(1, "free_tmp_dnode_info at %p\n", x);
	kmem_cache_free(tmp_dnode_info_slab, x);
}

struct jffs3_raw_node_ref *jffs3_alloc_raw_node_ref(void)
{
	struct jffs3_raw_node_ref *ret = kmem_cache_alloc(raw_node_ref_slab, GFP_KERNEL);
	DBG_SALLOC(1, "alloc_raw_node_ref at %p\n", ret);
	return ret;
}

void jffs3_free_raw_node_ref(struct jffs3_raw_node_ref *x)
{
	DBG_SALLOC(1, "free_raw_node_ref at %p\n", x);
	kmem_cache_free(raw_node_ref_slab, x);
}

struct jffs3_node_frag *jffs3_alloc_node_frag(void)
{
	struct jffs3_node_frag *ret = kmem_cache_alloc(node_frag_slab, GFP_KERNEL);
	DBG_SALLOC(1, "alloc_node_frag at %p\n", ret);
	return ret;
}

void jffs3_free_node_frag(struct jffs3_node_frag *x)
{
	DBG_SALLOC(1, "free_node_frag at %p\n", x);
	kmem_cache_free(node_frag_slab, x);
}

struct jffs3_inode_cache *jffs3_alloc_inode_cache(void)
{
	struct jffs3_inode_cache *ret = kmem_cache_alloc(inode_cache_slab, GFP_KERNEL);
	DBG_SALLOC(1, "Allocated inocache at %p\n", ret);
	return ret;
}

void jffs3_free_inode_cache(struct jffs3_inode_cache *x)
{
	DBG_SALLOC(1, "Freeing inocache at %p\n", x);
	kmem_cache_free(inode_cache_slab, x);
}


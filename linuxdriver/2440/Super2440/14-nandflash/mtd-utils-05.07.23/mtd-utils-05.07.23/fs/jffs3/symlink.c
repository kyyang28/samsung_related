/*
 * JFFS3 -- Journalling Flash File System, Version 3.
 *
 * Copyright (C) 2001, 2002 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@infradead.org>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * JFFS2 Id: symlink.c,v 1.14 2004/11/16 20:36:12 dwmw2  Exp
 * $Id: symlink.c,v 3.3 2005/01/05 16:19:01 dedekind Exp $
 *
 */


#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include "nodelist.h"

static int jffs3_follow_link(struct dentry *dentry, struct nameidata *nd);
static void jffs3_put_link(struct dentry *dentry, struct nameidata *nd);

struct inode_operations jffs3_symlink_inode_operations =
{
	.readlink =	generic_readlink,
	.follow_link =	jffs3_follow_link,
	.put_link =	jffs3_put_link,
	.setattr =	jffs3_setattr
};

static int jffs3_follow_link(struct dentry *dentry, struct nameidata *nd)
{
	unsigned char *buf;
	buf = jffs3_getlink(JFFS3_SB_INFO(dentry->d_inode->i_sb), JFFS3_INODE_INFO(dentry->d_inode));
	nd_set_link(nd, buf);
	return 0;
}

static void jffs3_put_link(struct dentry *dentry, struct nameidata *nd)
{
	char *s = nd_get_link(nd);
	if (!IS_ERR(s))
		kfree(s);
}

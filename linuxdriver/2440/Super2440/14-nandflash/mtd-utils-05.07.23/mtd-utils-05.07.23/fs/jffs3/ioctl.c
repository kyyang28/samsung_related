/*
 * JFFS3 -- Journalling Flash File System, Version 3.
 *
 * Copyright (C) 2001-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@infradead.org>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * JFFS2 Id: ioctl.c,v 1.9 2004/11/16 20:36:11 dwmw2  Exp
 * $Id: ioctl.c,v 3.3 2005/01/05 16:19:00 dedekind Exp $
 *
 */

#include <linux/fs.h>

int jffs3_ioctl(struct inode *inode, struct file *filp, unsigned int cmd,
		unsigned long arg)
{
	/* Later, this will provide for lsattr.jffs3 and chattr.jffs3, which
	   will include compression support etc. */
	return -ENOTTY;
}


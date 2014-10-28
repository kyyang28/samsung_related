/*
 * JFFS3 -- Journalling Flash File System, Version 3.
 *
 * Copyright (C) 2001, 2002 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@infradead.org>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * JFFS2 Id: writev.c,v 1.6 2004/11/16 20:36:12 dwmw2  Exp
 * $Id: writev.c,v 3.3 2005/01/07 08:54:00 havasi Exp $
 *
 */

#include <linux/kernel.h>
#include <linux/mtd/mtd.h>
#include "nodelist.h"
#include "summary.h"

/* This ought to be in core MTD code. All registered MTD devices
   without writev should have this put in place. Bug the MTD
   maintainer */
static inline int mtd_fake_writev(struct mtd_info *mtd, const struct kvec *vecs,
				  unsigned long count, loff_t to, size_t *retlen)
{
	unsigned long i;
	size_t totlen = 0, thislen;
	int ret = 0;

	for (i=0; i<count; i++) {
		if (!vecs[i].iov_len)
			continue;
		ret = mtd->write(mtd, to, vecs[i].iov_len, &thislen, vecs[i].iov_base);
		totlen += thislen;
		if (ret || thislen != vecs[i].iov_len)
			break;
		to += vecs[i].iov_len;
	}
	if (retlen)
		*retlen = totlen;
	return ret;
}

int jffs3_flash_direct_writev(struct jffs3_sb_info *c, const struct kvec *vecs,
			      unsigned long count, loff_t to, size_t *retlen)
{
	if (c->mtd->writev)
		return c->mtd->writev(c->mtd, vecs, count, to, retlen);
	else {
		
#ifdef CONFIG_JFFS3_SUMMARY
 		DBG_SUMMARY(1,"Without NAND support\n");
 		if (jffs3_sum_add_kvec(c,vecs,count,(uint32_t) to)) {
 			ERROR_MSG("MEMORY ALLOCATION ERROR!\n");
 		}	
#endif
		return mtd_fake_writev(c->mtd, vecs, count, to, retlen);
	}
}

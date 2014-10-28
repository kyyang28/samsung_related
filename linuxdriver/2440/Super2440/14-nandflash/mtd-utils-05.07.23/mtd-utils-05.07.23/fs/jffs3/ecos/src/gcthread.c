/*
 * JFFS3 -- Journalling Flash File System, Version 3.
 *
 * Copyright (C) 2001-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@infradead.org>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * JFFS2 Id: gcthread.c,v 1.2 2004/11/16 20:36:13 dwmw2  Exp
 * $Id: gcthread.c,v 3.3 2005/01/05 16:19:01 dedekind Exp $
 *
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include "jffs3.h"
#include "nodelist.h"


static void jffs3_garbage_collect_thread(struct jffs3_sb_info *c);

void jffs3_garbage_collect_trigger(struct jffs3_sb_info *c)
{
	/* Wake up the thread */
	(void)&jffs3_garbage_collect_thread;
}

void jffs3_start_garbage_collect_thread(struct jffs3_sb_info *c)
{
	/* Start the thread. Doesn't matter if it fails -- it's only an optimisation anyway */
}

void jffs3_stop_garbage_collect_thread(struct jffs3_sb_info *c)
{
	/* Stop the thread and wait for it if necessary */
}


static void jffs3_garbage_collect_thread(struct jffs3_sb_info *c)
{
#define this_thread_should_die() 0
	while(!this_thread_should_die()) {
		while(!jffs3_thread_should_wake(c)) {
			/* Sleep.... */
			continue;
		}
		if (jffs3_garbage_collect_pass(c) == -ENOSPC) {
			printf("No space for garbage collection. Aborting JFFS3 GC thread\n");
			break;
		}
	}
}

/*
 * JFFS3 -- Journalling Flash File System, Version 3.
 *
 * Copyright (C) 2001-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@infradead.org>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * JFFS2 Id: background.c,v 1.50 2004/11/16 20:36:10 dwmw2  Exp
 * $Id: background.c,v 3.8 2005/01/05 16:18:59 dedekind Exp $
 *
 */

#include <linux/kernel.h>
#include "jffs3.h"
#include <linux/mtd/mtd.h>
#include <linux/completion.h>
#include <linux/suspend.h>
#include "nodelist.h"


static int jffs3_garbage_collect_thread(void *);

void jffs3_garbage_collect_trigger(struct jffs3_sb_info *c)
{
	spin_lock(&c->erase_completion_lock);
        if (c->gc_task && jffs3_thread_should_wake(c))
                send_sig(SIGHUP, c->gc_task, 1);
	spin_unlock(&c->erase_completion_lock);
}

/* This must only ever be called when no GC thread is currently running */
int jffs3_start_garbage_collect_thread(struct jffs3_sb_info *c)
{
	pid_t pid;
	int ret = 0;

	if (c->gc_task)
		BUG();

	init_MUTEX_LOCKED(&c->gc_thread_start);
	init_completion(&c->gc_thread_exit);

	pid = kernel_thread(jffs3_garbage_collect_thread, c, CLONE_FS|CLONE_FILES);
	if (pid < 0) {
		WARNING_MSG("fork failed for JFFS3 garbage collect thread: %d\n", -pid);
		complete(&c->gc_thread_exit);
		ret = pid;
	} else {
		/* Wait for it... */
		DBG_GCT(1, "Garbage collect thread is pid %d\n", pid);
		down(&c->gc_thread_start);
	}

	return ret;
}

void jffs3_stop_garbage_collect_thread(struct jffs3_sb_info *c)
{
	spin_lock(&c->erase_completion_lock);
	if (c->gc_task) {
		DBG_GCT(1, "Killing GC task %d\n", c->gc_task->pid);
		send_sig(SIGKILL, c->gc_task, 1);
	}
	spin_unlock(&c->erase_completion_lock);
	wait_for_completion(&c->gc_thread_exit);
}

static int jffs3_garbage_collect_thread(void *_c)
{
	struct jffs3_sb_info *c = _c;

	daemonize("jffs3_gcd_mtd%d", c->mtd->index);
	allow_signal(SIGKILL);
	allow_signal(SIGSTOP);
	allow_signal(SIGCONT);

	c->gc_task = current;
	up(&c->gc_thread_start);

	set_user_nice(current, 10);

	for (;;) {
		allow_signal(SIGHUP);

		if (!jffs3_thread_should_wake(c)) {
			set_current_state (TASK_INTERRUPTIBLE);
			DBG_GCT(1, "GC thread sleeping...\n");
			/* Yes, there's a race here; we checked jffs3_thread_should_wake()
			   before setting current->state to TASK_INTERRUPTIBLE. But it doesn't
			   matter - We don't care if we miss a wakeup, because the GC thread
			   is only an optimisation anyway. */
			schedule();
		}

		if (current->flags & PF_FREEZE) {
			refrigerator(0);
			/* refrigerator() should recalc sigpending for us
			   but doesn't. No matter - allow_signal() will. */
			continue;
		}

		cond_resched();

		/* Put_super will send a SIGKILL and then wait on the sem.
		 */
		while (signal_pending(current)) {
			siginfo_t info;
			unsigned long signr;

			signr = dequeue_signal_lock(current, &current->blocked, &info);

			switch(signr) {
			case SIGSTOP:
				DBG_GCT(1, "SIGSTOP received.\n");
				set_current_state(TASK_STOPPED);
				schedule();
				break;

			case SIGKILL:
				DBG_GCT(1, "SIGKILL received.\n");
				goto die;

			case SIGHUP:
				DBG_GCT(1, "SIGHUP received.\n");
				break;
			default:
				DBG_GCT(1, "signal %ld received\n", signr);
			}
		}
		/* We don't want SIGHUP to interrupt us. STOP and KILL are OK though. */
		disallow_signal(SIGHUP);

		DBG_GCT(1, "GC pass\n");
		if (jffs3_garbage_collect_pass(c) == -ENOSPC) {
			WARNING_MSG("No space for garbage collection. Aborting GC thread\n");
			goto die;
		}
	}
 die:
	spin_lock(&c->erase_completion_lock);
	c->gc_task = NULL;
	spin_unlock(&c->erase_completion_lock);
	complete_and_exit(&c->gc_thread_exit, 0);
}

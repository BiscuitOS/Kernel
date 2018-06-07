/*
 * System Call: setpgid
 *
 * (C) 2018.03 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#define __LIBRARY__
#include <unistd.h>

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fs.h>

#include <asm/segment.h>

#include <sys/types.h>
#include <test/debug.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/*
 * This need some have checking ...
 * I just haven't get the stomach for it. I also don't fully
 * underftand sessions/pgrp etc. Let someboby who does explain it.
 */
int sys_d_setpgid(int pid, int pgid)
{
    int i;

    if (!pid)
        pid = current->pid;
    if (!pgid)
        pgid = current->pid;
    for (i = 0; i < NR_TASKS; i++)
        if (task[i] && task[i]->pid == pid) {
            if (task[i]->leader)
                return -EPERM;
            if (task[i]->session != current->session)
                return -EPERM;
            task[i]->pgrp = pgid;
            return 0;
        }
    return -ESRCH;
}

/* Invoke by system call: int $0x80 */
int debug_syscall_setpgid0(void)
{
    d_setpgid(1, 1);
    return 0;
}

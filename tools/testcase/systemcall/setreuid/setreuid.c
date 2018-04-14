/*
 * System Call: setreuid
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
 * Unprivileged users may change the real user id to the effective uid
 * or vice versa.
 */
int sys_d_setreuid(int ruid, int euid)
{
    int old_ruid = current->uid;

    if (ruid > 0) {
        if ((current->euid == ruid) ||
            (old_ruid == ruid) ||
            suser())
            current->uid = ruid;
        else
            return -EPERM;
    }
    if (euid > 0) {
        if ((old_ruid == euid) ||
            (current->euid == euid) ||
             suser())
            current->euid = euid;
        else {
            current->uid = old_ruid;
            return -EPERM;
        }
    }
    return 0;
}

/* Invoke by system call: int $0x80 */
int debug_syscall_setreuid0(void)
{
    d_setreuid(1, 1);
    return 0;
}

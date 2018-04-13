/*
 * System Call: setgid
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

int sys_d_setregid(int rgid, int egid)
{
    if (rgid > 0) {
        if ((current->gid == rgid) ||
               suser())
            current->gid = rgid;
        else
            return -EPERM;
    }
    if (egid > 0) {
        if ((current->gid == egid)  ||
            (current->egid == egid) ||
            (current->sgid == egid) ||
             suser())
            current->egid = egid;
        else
            return -EPERM;
    }
    return 0;
}

int sys_d_setgid(int gid)
{
    return sys_d_setregid(gid, gid);
}

/* Invoke by system call: int $0x80 */
int debug_syscall_setgid0(void)
{
    d_setgid(2);
    return 0;
}

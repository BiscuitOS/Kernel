/*
 * System Call: stime
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

int sys_d_stime(long *tptr)
{
    if (!suser())
        return -EPERM;
    startup_time = get_fs_long((unsigned long *)tptr) - jiffies / HZ;
    return 0;
}

/* Invoke by system call: int $0x80 */
int debug_syscall_stime0(void)
{
    long time = 20;
    
    d_stime(&time);
    
    return 0;
}

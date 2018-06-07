/*
 * System Call: utime
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

#include <test/debug.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int sys_d_utime(const char *filename, struct utimbuf *times)
{
    struct m_inode *inode;
    long actime, modtime;

    if (!(inode = namei(filename)))
        return -ENOENT;
    if (times) {
        actime = get_fs_long((unsigned long *)&times->actime);
        modtime = get_fs_long((unsigned long *)&times->modtime);
    } else
        actime = modtime = CURRENT_TIME;

    inode->i_atime = actime;
    inode->i_mtime = modtime;
    inode->i_dirt  = 1;
    iput(inode);
    return 0;
}

/* Invoke by system call: int $0x80 */
int debug_syscall_utime0(void)
{
    struct utimbuf times;

    times.actime = 10;
    times.modtime = 20;

    d_utime("/etc/rc", &times);
    return 0;
}

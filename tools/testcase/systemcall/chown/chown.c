/*
 * System Call: chown
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

#include <test/debug.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int sys_d_chown(const char *filename, int uid, int gid)
{
    struct m_inode *inode;

    if (!(inode = namei(filename)))
        return -ENOENT;
    if (!suser()) {
        iput(inode);
        return -EACCES;
    }
    inode->i_uid = uid;
    inode->i_gid = gid;
    inode->i_dirt = 1;
    iput(inode);
    return 0;
}

/* Invoke by system call: int $0x80 */
int debug_syscall_chown0(void)
{
    d_chown("/etc/rc", 0755, 0755);
    return 0;
}

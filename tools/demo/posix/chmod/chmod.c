/*
 * System Call: chmod
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

int sys_d_chmod(const char *filename, int mode)
{
    struct m_inode *inode;

    if (!(inode = namei(filename)))
        return -ENOENT;
    if ((current->euid != inode->i_uid) & !suser()) {
        iput(inode);
        return -EACCES;
    }
    inode->i_mode = (mode & 07777) | (inode->i_mode & ~07777);
    inode->i_dirt = 1;
    iput(inode);
    return 0;
}

/* Invoke by system call: int $0x80 */
int debug_syscall_chmod0(void)
{
    d_chmod("/etc/rc", 0777);
    return 0;
}

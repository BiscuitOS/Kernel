/*
 * System Call: close
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

extern void d_iput(struct m_inode *inode);

int sys_d_close(unsigned int fd)
{
    struct file *filp;

    if (fd >= NR_OPEN)
        return -EINVAL;
    current->close_on_exec &= ~(1 << fd);
    if (!(filp = current->filp[fd]))
        return -EINVAL;
    current->filp[fd] = NULL;
    if (filp->f_count == 0)
        panic("Close: file count is 0");
    if (--filp->f_count)
        return 0;
    d_iput(filp->f_inode);
    return 0;
}

/* Invoke by system call: int $0x80 */
int debug_syscall_close0(void)
{
    int fd;

    fd = open("/etc/biscuitos.conf", O_RDWR | O_CREAT);

    d_close(fd);
    return 0;
}

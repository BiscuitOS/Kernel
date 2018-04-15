/*
 * System Call: dup2
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

static int dupfd(unsigned int fd, unsigned int arg)
{
    if (fd >= NR_OPEN || !current->filp[fd])
        return -EBADF;
    if (arg >= NR_OPEN)
        return -EINVAL;
    while (arg < NR_OPEN)
        if (current->filp[arg])
            arg++;
        else
            break;
    if (arg >= NR_OPEN)
        return -EMFILE;
    current->close_on_exec &= ~(1 << arg);
    (current->filp[arg] = current->filp[fd])->f_count++;
    return arg;
}

static int sys_d_close(unsigned int fd)
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
    iput(filp->f_inode);
    return 0;
}

int sys_d_dup2(unsigned int oldfd, unsigned int newfd)
{
    sys_d_close(newfd);
    return dupfd(oldfd, newfd);
}

/* Invoke by system call: int $0x80 */
int debug_syscall_dup20(void)
{
    int fd;
    int dupfd;

    fd = open("/ect/rc", O_RDWR);

    dupfd = d_dup2(fd, fd + 1);
    printf("Dupfd: %#x\n", dupfd);

    close(fd);
    return 0;
}

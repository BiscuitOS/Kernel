/*
 * System Call: fcntl
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

int sys_d_fcntl(unsigned int fd, unsigned int cmd, unsigned long arg)
{
    struct file *filp;

    if (fd >= NR_OPEN || !(filp = current->filp[fd]))
        return -EBADF;
    switch (cmd) {
    case F_DUPFD:
        return dupfd(fd, arg);
    case F_GETFD:
        return (current->close_on_exec >> fd) & 1;
    case F_SETFD:
        if (arg & 1)
            current->close_on_exec |= (1 << fd);
        else
            current->close_on_exec &= ~(1 << fd);
        return 0;
    case F_GETFL:
        return filp->f_flags;
    case F_SETFL:
        filp->f_flags &= ~(O_APPEND | O_NONBLOCK);
        filp->f_flags |= arg & (O_APPEND | O_NONBLOCK);
        return 0;
    case F_GETLK:
    case F_SETLK:
    case F_SETLKW:
        return -1;
    default:
        return -1;
    }
}

/* Invoke by system call: int $0x80 */
int debug_syscall_fcntl0(void)
{
    int fd;
    int flags;

    fd = open("/ect/rc", O_RDWR);

    flags = d_fcntl(fd, F_GETFL, 1);
    printf("Flags: %#x\n", flags);

    close(fd);
    return 0;
}

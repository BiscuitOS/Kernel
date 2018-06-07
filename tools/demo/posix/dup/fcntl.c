/*
 * fcntl.c
 *
 * (C) 2018.03 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/sched.h>

#include <errno.h>

int dupfd(unsigned int fd, unsigned int arg)
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

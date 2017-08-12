/*
 * linux/fs/fcntl.c
 *
 * (C) 1991 Linus Torvalds
 */

#include <linux/fs.h>
#include <linux/sched.h>

#include <errno.h>

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

int sys_dup(unsigned int fildes)
{
    return dupfd(fildes, 0);
}

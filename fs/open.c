/*
 * linux/fs/open.c
 *
 * (C) 1991 Linus Torvalds
 */
#include <linux/fs.h>
#include <linux/sched.h>

#include <errno.h>

int sys_close(unsigned int fd)
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

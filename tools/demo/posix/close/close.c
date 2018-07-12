/*
 * POSIX system call: close
 *
 * (C) 2018.07.12 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/stat.h>

#include <demo/debug.h>

/* System Clib */
static inline _syscall3(int, open, const char *, file, int, flag, int, mode);

extern void fcntl_remove_locks(struct task_struct *, struct file *, 
                              unsigned int fd);

static int close_fds(struct file *filp, unsigned int fd)
{
    struct inode *inode;

    if (filp->f_count == 0) {
        printk("VFS: Close: file count is 0\n");
        return 0;
    }
    inode = filp->f_inode;
    if (inode && S_ISREG(inode->i_mode))
        fcntl_remove_locks(current, filp, fd);
    if (filp->f_count > 1) {
        filp->f_count--;
        return 0;
    }
    if (filp->f_op && filp->f_op->release)
        filp->f_op->release(inode, filp);
    filp->f_count--;
    filp->f_inode = NULL;
    iput(inode);
    return 0;
}

asmlinkage int sys_demo_close(unsigned int fd)
{
    struct file *filp;

    if (fd >= NR_OPEN)
        return -EBADF;
    FD_CLR(fd, &current->close_on_exec);
    if (!(filp = current->filp[fd]))
        return -EBADF;
    current->filp[fd] = NULL;
    return (close_fds(filp, fd));
}

/* build a system call entry for write */
inline _syscall1(int, demo_close, unsigned int, fd);

/* userland code */
static int debug_close(void)
{
    int fd;

    fd = open("/etc/rc",  O_RDONLY, 0);
    if (fd < 0) {
        printf("Can't open /etc/rc on demo_close\n");
        return -EINVAL;
    }

#ifdef CONFIG_CLOSE_ORIG
    close(fd);
#else
    demo_close(fd);
#endif
    return 0;
}
user1_debugcall_sync(debug_close);

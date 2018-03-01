/*
 * System Call: open
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

#include <errno.h>

extern int d_open_namei(const char *pathname, int flag, int mode,
                 struct m_inode **res_inode);

/* system call: d_open */
int sys_d_open(const char *filename, int flag, int mode)
{
    struct m_inode *inode;
    struct file *f;
    int fd, i;
 
    for (fd = 0; fd < NR_OPEN; fd++)
        if (!current->filp[fd])
            break;
    if (fd >= NR_OPEN)
        return -EINVAL;

    current->close_on_exec &= ~(1 << fd);
    f = 0 + file_table;
    for (i = 0; i < NR_FILE; i++, f++)
        if (!f->f_count)
            break;
    if (i >= NR_FILE)
        return -EINVAL;

    (current->filp[fd] = f)->f_count++;
    if ((i = d_open_namei(filename, flag, mode, &inode)) < 0) {
        current->filp[fd] = NULL;
        f->f_count = 0;
        return i;
    }
    return 0;
}

/* Invoke by system call: int $0x80 */
int debug_syscall_open0(void)
{
    d_open("/etc/profile", 0, 0);
    return 0;
}

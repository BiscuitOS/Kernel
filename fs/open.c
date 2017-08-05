/*
 * linux/fs/open.c
 *
 * (C) 1991 Linus Torvalds
 */
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/tty.h>

#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>

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

int sys_open(const char *filename, int flag, int mode)
{
    struct m_inode *inode;
    struct file *f;
    int i, fd;

    mode &= 0777 & ~current->umask;
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
    if ((i = open_namei(filename, flag, mode, &inode)) < 0) {
        current->filp[fd] = NULL;
        f->f_count = 0;
        return i;
    }
    /* ttys are somewhat special (ttyxx major == 4, tty major == 5) */
    if (S_ISCHR(inode->i_mode)) {
        if (MAJOR(inode->i_zone[0]) == 4) {
            if (current->leader && current->tty < 0) {
                current->tty = MINOR(inode->i_zone[0]);
                tty_table[current->tty].pgrp = current->pgrp;
            }
        } else if (MAJOR(inode->i_zone[0]) == 5)
            if (current->tty < 0) {
                iput(inode);
                current->filp[fd] = NULL;
                f->f_count = 0;
                return -EPERM;
            }
    }
    /* Likewise with block-devices: check for floppy_change */
    if (S_ISBLK(inode->i_mode))
        check_disk_change(inode->i_zone[0]);
    f->f_mode  = inode->i_mode;
    f->f_flags = flag;
    f->f_count = 1;
    f->f_inode = inode;
    f->f_pos   = 0;
    return (fd);
}

int sys_creat(const char *pathname, int mode)
{
    return sys_open(pathname, O_CREAT | O_TRUNC, mode);
}

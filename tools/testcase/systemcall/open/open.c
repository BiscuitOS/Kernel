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
#include <fcntl.h>

extern int d_open_namei(const char *pathname, int flag, int mode,
                 struct m_inode **res_inode);

/* system call: d_open 
 *
 * @filename: the file name.
 * @flag: RD/WR/CRT
 * @mode:
 *
 * @return: fd
 */
int sys_d_open(const char *filename, int flag, int mode)
{
    struct m_inode *inode;
    struct file *f;
    int fd, i;
 
    mode &= 0777 & ~current->umask;
    /* Obtain a valid fd from current task */
    for (fd = 0; fd < NR_OPEN; fd++)
        if (!current->filp[fd])
            break;
    if (fd >= NR_OPEN)
        return -EINVAL;

    /* Set close mask */
    current->close_on_exec &= ~(1 << fd);
    /* Obtain a valid structure for file */
    f = 0 + file_table;
    for (i = 0; i < NR_FILE; i++, f++)
        if (!f->f_count)
            break;
    if (i >= NR_FILE)
        return -EINVAL;

    (current->filp[fd] = f)->f_count++;
    /* Obtain special inode structure for file */
    if ((i = d_open_namei(filename, flag, mode, &inode)) < 0) {
        current->filp[fd] = NULL;
        f->f_count = 0;
        return i;
    }

    /* ttys are somewhat special (ttyxx major == 4, tty major == 5) */
    if (S_ISCHR(inode->i_mode)) {
        if (MAJOR(inode->i_zone[0]) == 4) {
            if (current->leader && current->tty < 0) {
                /* Nothing routine */
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
        /* Noting to do this routine */;
    f->f_mode   = inode->i_mode;
    f->f_flags  = flag;
    f->f_count  = 1;
    f->f_inode  = inode;
    f->f_pos    = 0;
    return (fd);
}

/* Invoke by system call: int $0x80 */
int debug_syscall_open0(void)
{
    int fd;
    int fd0, fd1, fd2, fd3;

    /* Open a exist file */
    fd = d_open("/etc/profile", O_RDWR, 0);
    /* Open '.' */
    fd0 = d_open(".", O_RDWR, 0);
    /* Open ".." */
    fd1 = d_open("..", O_RDWR, 0);
    /* Create a file */
    fd2 = d_open("/etc/biscuitos.conf", O_CREAT | O_RDWR);
    /* Open /dev/ttyX */
    fd3 = d_open("/dev/tty0", O_RDONLY, 0);

    /* Other operation */
    write(fd2, "BiscuitOS\n", 10);

    /* close file */
    close(fd3);
    close(fd2);
    close(fd1);
    close(fd0);
    close(fd);
    return 0;
}

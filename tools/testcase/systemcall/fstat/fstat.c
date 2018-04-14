/*
 * System Call: fstat
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

static void cp_stat(struct m_inode *inode, struct stat *statbuf)
{
    struct stat tmp;
    int i;

    verify_area(statbuf, sizeof(*statbuf));
    tmp.st_dev     = inode->i_dev;
    tmp.st_ino     = inode->i_num;
    tmp.st_mode    = inode->i_mode;
    tmp.st_nlink   = inode->i_nlinks;
    tmp.st_uid     = inode->i_uid;
    tmp.st_gid     = inode->i_gid;
    tmp.st_rdev    = inode->i_zone[0];
    tmp.st_size    = inode->i_size;
    tmp.st_atime   = inode->i_atime;
    tmp.st_mtime   = inode->i_mtime;
    tmp.st_ctime   = inode->i_ctime;
    for (i = 0; i < sizeof(tmp); i++)
        put_fs_byte(((char *)&tmp)[i], &((char *)statbuf)[i]);
}

int sys_d_fstat(unsigned int fd, struct stat *statbuf)
{
    struct file *f;
    struct m_inode *inode;

    if (fd >= NR_OPEN || !(f = current->filp[fd]) || !(inode = f->f_inode))
        return -EBADF;
    cp_stat(inode, statbuf);
    return 0;
}

/* Invoke by system call: int $0x80 */
int debug_syscall_fstat0(void)
{
    int fd;
    struct stat statbuf;

    fd = open("/etc/rc", O_RDWR);
    d_fstat(fd, &statbuf);

    printf("Block %d\n", statbuf.st_rdev);

    close(fd);
    return 0;
}

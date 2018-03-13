/*
 * System Call: write
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
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

extern int d_file_write(struct m_inode *inode, struct file *filp, char *buf,
     int count);

int sys_d_write(unsigned int fd, char *buf, int count)
{
    struct file *file;
    struct m_inode *inode;

    if (fd >= NR_OPEN || count < 0 || !(file = current->filp[fd]))
        return -EINVAL;
    inode = file->f_inode;
    if (S_ISREG(inode->i_mode))
        return d_file_write(inode, file, buf, count);
    printk("(Write)inode->i_mode=%060\n\r", inode->i_mode);
    return -EINVAL;
}

/* Invoke by system call: int $0x80 */
int debug_syscall_write0(void)
{
    int fd;
    char buf[20] = "BiscuitOS-Anna\n";

    fd = open("/etc/biscuitos.conf", O_RDWR | O_CREAT, 0);
    if (!fd) {
        d_printf("Unable to open /etc/biscuitos.conf\n");
        return -1;
    }

    /* write data into file */
    d_write(fd, buf, strlen(buf));

    /* Close file */
    close(fd);
    return 0;
}

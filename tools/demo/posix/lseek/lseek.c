/*
 * System Call: lseek
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

#define SEEK_SET      0   /* Seek from beginning of file */
#define SEEK_CUR      1   /* Seek from current position */
#define SEEK_END      2   /* Set file pointer to EOF plus "offset" */

int sys_d_lseek(unsigned int fd, off_t offset, int origin)
{
    struct file *file;
    int tmp;

    if (fd >= NR_OPEN || !(file = current->filp[fd]) || !(file->f_inode)
        || !IS_SEEKABLE(MAJOR(file->f_inode->i_dev)))
        return -EBADF;
    if (file->f_inode->i_pipe)
        return -ESPIPE;
    switch (origin) {
    case 0:
        if (offset < 0)
            return -EINVAL;
        file->f_pos = offset;
        break;
    case 1:
        if (file->f_pos + offset < 0)
            return -EINVAL;
        file->f_pos += offset;
        break;
    case 2:
        if ((tmp = file->f_inode->i_size + offset) < 0)
            return -EINVAL;
        file->f_pos = tmp;
        break;
    default:
        return -EINVAL;
    }
    return file->f_pos;
}

/* Invoke by system call: int $0x80 */
int debug_syscall_lseek0(void)
{
    int fd, offset;

    fd = open("/etc/rc", O_RDWR);

    /* Seek from beginning of file */
    offset = d_lseek(fd, 0, SEEK_SET);
    printf("Start:   %#x\n", offset);

    /* Seek from current position */
    offset = d_lseek(fd, 10, SEEK_CUR);
    printf("Current: %#x\n", offset);

    /* Set file pointer to EOF plus "offset" */
    offset = d_lseek(fd, 0, SEEK_END);
    printf("END:     %#x\n", offset);
    
    close(fd);
    return 0;
}

/*
 * file_dev.c
 *
 * (C) 2018.03 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fs.h>

#include <asm/segment.h>

#include <errno.h>

#define MIN(a,b) (((a)<(b))?(a):(b))

extern int d_bmap(struct m_inode *inode, int block);

int d_file_read(struct m_inode *inode, struct file *filp, char *buf, 
                 int count)
{
    int left, chars, nr;
    struct buffer_head *bh;

    if ((left = count) <= 0)
        return 0;
    while (left) {
        if ((nr = d_bmap(inode, (filp->f_pos) / BLOCK_SIZE))) {
            if (!(bh = bread(inode->i_dev, nr)))
                break;
        } else
            bh = NULL;
        nr = filp->f_pos % BLOCK_SIZE;
        chars = MIN(BLOCK_SIZE - nr, left);
        filp->f_pos += chars;
        left -= chars;
        if (bh) {
            char *p = nr + bh->b_data;

            while (chars-- > 0)
                put_fs_byte(*(p++), buf++);
            brelse(bh);
        } else {
            while (chars-- > 0)
                put_fs_byte(0, buf++);
        }
    }
    inode->i_atime = CURRENT_TIME;
    return (count - left) ? (count - left) : -ERROR;
}

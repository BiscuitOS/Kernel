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
#include <linux/fs.h>
#include <linux/sched.h>

#include <asm/segment.h>
#include <fcntl.h>

extern int d_create_block(struct m_inode *inode, int block);

int d_file_write(struct m_inode *inode, struct file *filp, char *buf, 
     int count)
{
    off_t pos;
    int block, c;
    struct buffer_head *bh;
    char *p;
    int i = 0;

    /*
     * Ok, append may not work when many processes are writing at the
     * same time but so what. That way leads to madness anyway.
     */
    if (filp->f_flags & O_APPEND)
        pos = inode->i_size;
    else
        pos = filp->f_pos;
    while (i < count) {
        if (!(block = create_block(inode, pos / BLOCK_SIZE)))
            break;
        if (!(bh = bread(inode->i_dev, block)))
            break;
        c = pos % BLOCK_SIZE;
        p = c + bh->b_data;
        bh->b_dirt = 1;
        c = BLOCK_SIZE - c;
        if (c > count - i)
            c = count - i;
        pos += c;
        if (pos > inode->i_size) {
            inode->i_size = pos;
            inode->i_dirt = 1;
        }
        i += c;
        while (c-- > 0)
            *(p++) = get_fs_byte(buf++);
        brelse(bh);
    }
    inode->i_mtime = CURRENT_TIME;
    if (!(filp->f_flags & O_APPEND)) {
        filp->f_pos = pos;
        inode->i_ctime = CURRENT_TIME;
    }
    return (i ? i : -1);
}

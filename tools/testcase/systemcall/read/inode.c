/*
 * inode.c
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

static int _bmap(struct m_inode *inode, int block, int create)
{
    struct buffer_head *bh;
    int i;

    if (block < 0)
        panic("_bmp: block < 0");
    if (block >= 7 + 512 + 512 + 512)
        panic("_bmp: block > big");
    if (block < 7) {
        if (create && !inode->i_zone[block])
            if ((inode->i_zone[block] = new_block(inode->i_dev))) {
                inode->i_ctime = CURRENT_TIME;
                inode->i_dirt = 1;
            }
        return inode->i_zone[block];
    }
    return 0;
}

int d_bmap(struct m_inode *inode, int block)
{
    return _bmap(inode, block, 0);
}

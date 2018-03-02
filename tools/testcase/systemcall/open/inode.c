/*
 * inode.c
 *
 * (C) 2018.03 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

static int _bmap(struct m_inode *inode, int block, int create)
{
    struct buffer_head *bh;

    if (block < 0)
        panic("_bmp: block < 0");
}

int d_map(struct m_inode *inode, int block)
{
    return _bmap(inode, block, 0);
}

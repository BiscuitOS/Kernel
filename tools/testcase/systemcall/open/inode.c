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

static int _bmap(struct m_inode *inode, int block, int create)
{
    struct buffer_head *bh;

    printk("Hello world\n");
    if (block < 0)
        panic("_bmp: block < 0");
}

int d_map(struct m_inode *inode, int block)
{
    return _bmap(inode, block, 0);
}

static struct m_inode *d_get_empty_inode(void)
{
    struct m_inode *inode;
    static struct m_inode *last_inode = inode_table;
    int i;

    do {
        inode = NULL;
        for (i = NR_INODE; i; i--) {
        
        }
    } while (inode->i_count); 
    return NULL;
}

struct m_inode *d_iget(int dev, int nr)
{
    struct m_inode *inode, *empty;

    if (!dev)
        panic("iget with dev==0");
    empty = d_get_empty_inode();
    return NULL;
}

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
#include <asm/system.h>
#include <linux/sched.h>
#include <linux/mm.h>

#include <sys/stat.h>

#include <string.h>
extern void d_free_inode(struct m_inode *inode);

static struct m_inode d_inode_table[NR_INODE] = {{0, }, };

static inline void wait_on_inode(struct m_inode *inode)
{
    cli();
    while (inode->i_lock)
        sleep_on(&inode->i_wait);
    sti();
}

static inline void lock_inode(struct m_inode *inode)
{
    cli();
    while (inode->i_lock)
        sleep_on(&inode->i_wait);
    inode->i_lock = 1;
    sti();
}

static inline void unlock_inode(struct m_inode *inode)
{
    inode->i_lock = 0;
    wake_up(&inode->i_wait);
}

static int _bmap(struct m_inode *inode, int block, int create)
{
    if (block < 0)
        panic("_bmp: block < 0");
    return 0;
}

int d_map(struct m_inode *inode, int block)
{
    return _bmap(inode, block, 0);
}

static void write_inode(struct m_inode *inode)
{
    struct super_block *sb;
    struct buffer_head *bh;
    int block;

    lock_inode(inode);
    if (!inode->i_dirt || !inode->i_dev) {
        unlock_inode(inode);
        return;
    }
    if (!(sb = get_super(inode->i_dev)))
        panic("trying to write inode without device");
    block = 2 + sb->s_imap_blocks + sb->s_zmap_blocks +
            (inode->i_num - 1) / INODES_PER_BLOCK;
    if (!(bh = bread(inode->i_dev, block)))
        panic("unable to read i-node block");
    ((struct d_inode *)bh->b_data)
             [(inode->i_num - 1) % INODES_PER_BLOCK] =
                  *(struct d_inode *)inode;
    bh->b_dirt = 1;
    inode->i_dirt = 0;
    brelse(bh);
    unlock_inode(inode);
}

static void read_inode(struct m_inode *inode)
{
    struct super_block *sb;
    struct buffer_head *bh;
    int block;

    lock_inode(inode);
    if (!(sb = get_super(inode->i_dev)))
        panic("trying to read inode without dev");
    block = 2 + sb->s_imap_blocks + sb->s_zmap_blocks + 
            (inode->i_num - 1) /INODES_PER_BLOCK;
    if (!(bh = bread(inode->i_dev, block)))
        panic("unable to read i_inode block");
    *(struct d_inode *)inode = 
        ((struct d_inode *)bh->b_data)
           [(inode->i_num - 1) % INODES_PER_BLOCK];
    brelse(bh);
    unlock_inode(inode);
}

struct m_inode *d_get_empty_inode(void)
{
    struct m_inode *inode;
    static struct m_inode *last_inode = d_inode_table;
    int i;

    do {
        inode = NULL;
        for (i = NR_INODE; i; i--) {
            if (++last_inode >= d_inode_table + NR_INODE)
                last_inode = d_inode_table;
            if (!last_inode->i_count) {
                inode = last_inode;
                if (!inode->i_dirt && !inode->i_lock)
                    break;
            }
        }
        if (!inode) {
            for (i = 0; i < NR_INODE; i++)
                printk("%04x:%6d\t", d_inode_table[i].i_dev,
                                     d_inode_table[i].i_num);
            panic("No free inodes in mem");
        }
        wait_on_inode(inode);
        while (inode->i_dirt) {
            write_inode(inode);
            wait_on_inode(inode);
        }
    } while (inode->i_count); 
    memset(inode, 0, sizeof(*inode));
    inode->i_count = 1;
    return inode;
}

void d_iput(struct m_inode *inode)
{
    if (!inode)
        return;
    wait_on_inode(inode);
    if (!inode->i_count)
        panic("iput: trying to free free inode");
    if (inode->i_pipe) {
        wake_up(&inode->i_wait);
        if (--inode->i_count)
            return;
        free_page(inode->i_size);
        inode->i_count = 0;
        inode->i_dirt  = 0;
        inode->i_pipe  = 0;
        return;
    }
    if (!inode->i_dev) {
        inode->i_count--;
        return;
    }
    if (S_ISBLK(inode->i_mode)) {
        sync_dev(inode->i_zone[0]);
        wait_on_inode(inode);
    }
repeat:
    if (inode->i_count > 1) {
        inode->i_count--;
        return;
    }
    if (!inode->i_nlinks) {
        truncate(inode);
        d_free_inode(inode);
        return;
    }
    if (inode->i_dirt) {
        write_inode(inode);
        wait_on_inode(inode);
        goto repeat;
    }
    inode->i_count--;
    return;
}

struct m_inode *d_iget(int dev, int nr)
{
    struct m_inode *inode, *empty;

    if (!dev)
        panic("iget with dev==0");
    empty = d_get_empty_inode();
    inode = d_inode_table;
    while (inode < NR_INODE + d_inode_table) {
        if (inode->i_dev != dev || inode->i_num != nr) {
            inode++;
            continue;
        }
        wait_on_inode(inode);
        if (inode->i_dev != dev || inode->i_num != nr) {
            inode = d_inode_table;
            continue;
        }
        inode->i_count++;
        if (inode->i_mount) {
            int i;

            for (i = 0; i < NR_SUPER; i++)
                if (super_block[i].s_imount == inode)
                    break;
            if (i >= NR_SUPER) {
                printk("Mounted inode hasn't got sb\n");
                if (empty)
                    d_iput(empty);
                return inode;
            }
            d_iput(inode);
            dev = super_block[i].s_dev;
            nr = ROOT_INO;
            inode = d_inode_table;
            continue;
        }
        if (empty)
            d_iput(empty);
        return inode;
    }
    if (!empty)
        return NULL;
    inode = empty;
    inode->i_dev = dev;
    inode->i_num = nr;
    read_inode(inode);
    return inode;
}

int d_create_block(struct m_inode *inode, int block)
{
    return _bmap(inode, block, 1);
}

/*
 * bitmap.c
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
#include <string.h>

#include <test/debug.h>

#define clear_bit(nr, addr) ({\
    register int res; \
    __asm__ __volatile__("btrl %2, %3\n\tsetnb %%al": \
             "=a" (res) : "0" (0), "r" (nr), "m" (*(addr))); \
    res;})

#define set_bit(nr, addr) ({\
    register int res; \
    __asm__ __volatile__("btsl %2, %3\n\tsetb %%al": \
    "=a" (res) : "0" (0), "r" (nr), "m" (*(addr))); \
    res;})

#define find_first_zero(addr) ({ \
    int __res; \
    __asm__ __volatile__ ("cld\n" \
                          "1:\tlodsl\n\t" \
                          "notl %%eax\n\t" \
                          "bsfl %%eax, %%edx\n\t" \
                          "je 2f\n\t" \
                          "addl %%edx, %%ecx\n\t" \
                          "jmp 3f\n"  \
                          "2:\taddl $32, %%ecx\n\t" \
                          "cmpl $8192, %%ecx\n\t" \
                          "jl 1b\n" \
                          "3:" \
                          :"=c" (__res):"c" (0), "S" (addr)); \
__res;})

extern struct m_inode *d_get_empty_inode(void);
extern int d_iput(struct m_inode *inode);

void d_free_inode(struct m_inode *inode)
{
    struct super_block *sb;
    struct buffer_head *bh;

    if (!inode)
        return;
    if (!inode->i_dev) {
        memset(inode, 0, sizeof(*inode));
        return;
    }
    if (inode->i_count > 1) {
        printk("trying to free inode with count=%d\n", inode->i_count);
        panic("free_inode");
    }
    if (inode->i_nlinks)
        panic("trying to free inode with links");
    if (!(sb = get_super(inode->i_dev)))
        panic("t_rying to free inode with nonexistent device");
    if (inode->i_num < 1 || inode->i_num > sb->s_ninodes)
        panic("trying to free inode 0 or nonexistent inode");
    if (!(bh = sb->s_imap[inode->i_num >> 13]))
        panic("nonexistent imap in superblock");
    if (clear_bit(inode->i_num & 8191, bh->b_data))
        printk("free_inode: bit already cleared.\n\r");
    bh->b_dirt = 1;
    memset(inode, 0, sizeof(*inode));
}

struct m_inode *d_new_inode(int dev)
{
    struct m_inode *inode;
    struct super_block *sb;
    struct buffer_head *bh;
    int i, j;

    if (!(inode = d_get_empty_inode()))
        return NULL;
    if (!(sb = get_super(dev)))
        panic("new_inode with unknown device");
    j = 8192;
    for (i = 0; i < 8; i++)
        if ((bh = sb->s_imap[i]))
            if ((j = find_first_zero(bh->b_data)) < 8192)
                break;
    if (!bh || j >= 8192 || j + i * 8192 > sb->s_ninodes) {
        d_iput(inode);
        return NULL;
    }
    if (set_bit(j, bh->b_data))
        panic("d_new_inode: bit already set");
    bh->b_dirt = 1;
    inode->i_count = 1;
    inode->i_nlinks = 1;
    inode->i_dev = dev;
    inode->i_uid = current->euid;
    inode->i_gid = current->egid;
    inode->i_dirt = 1;
    inode->i_num = j + i * 8192;
    inode->i_mtime = inode->i_atime = inode->i_ctime = CURRENT_TIME;
    return inode;
}

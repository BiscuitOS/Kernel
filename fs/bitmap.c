/*
 * linux/fs/bitmap.c
 * 
 * (C) 1991 Linus Torvalds
 */

/* bitmap.c contains the code that handles the inode and block bitmaps */
#include <linux/fs.h>
#include <linux/kernel.h>

#include <string.h>

#define set_bit(nr, addr) ({\
register int res; \
__asm__ __volatile__("btsl %2,%3\n\tsetb %%al": \
"=a" (res): "0" (0), "r" (nr), "m" (*(addr))); \
res;})

#define clear_bit(nr, addr) ({\
register int res; \
__asm__ __volatile__("btrl %2,%3\n\tsetnb %%al": \
"=a" (res): "0" (0), "r" (nr), "m" (*(addr))); \
res;})

void free_inode(struct m_inode *inode)
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
        panic("trying to free inode with nonexistent device");
    if (inode->i_num < 1 || inode->i_num > sb->s_ninodes)
        panic("trying to free inode 0 or nonexistent inode");
    if (!(bh = sb->s_imap[inode->i_num >> 13]))
        panic("nonexistent imap in superblock");
    if (clear_bit(inode->i_num & 8191, bh->b_data))
        printk("free_inode: bit already cleared.\n\r");
    bh->b_dirt = 1;
    memset(inode, 0, sizeof(*inode));
}

void free_block(int dev, int block)
{
    struct super_block *sb;
    struct buffer_head *bh;

    if (!(sb = get_super(dev)))
        panic("trying to free block on nonexistent device");
    if (block < sb->s_firstdatazone || block >= sb->s_nzones)
        panic("trying to free block not in datazone");
    bh = get_hash_table(dev, block);
    if (bh) {
        if (bh->b_count != 1) {
            printk("trying to free block (%04x:%d), count=%d\n",
                    dev, block, bh->b_count);
            return;
        }
        bh->b_dirt = 0;
        bh->b_uptodate = 0;
        brelse(bh);
    }
    block -= sb->s_firstdatazone - 1;
    if (clear_bit(block & 8191, sb->s_zmap[block / 8192]->b_data)) {
        printk("block (%04x:%d) ", dev, block + sb->s_firstdatazone - 1);
        panic("free_block: bit already cleared");
    }
    sb->s_zmap[block / 8192]->b_dirt = 1;
}

/*
 * POSIX system call: write
 *
 * (C) 2018.06.18 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/stat.h>
#include <linux/locks.h>

#include <asm/segment.h>
#include <asm/bitops.h>

#define clear_block(addr) \
    __asm__("cld\n\t" \
            "rep\n\t" \
            "stosl" \
            : \
            : "a" (0), "c" (BLOCK_SIZE/4), "D" ((long)(addr)))

#define find_first_zero(addr) ({ \
    int __res; \
    __asm__("cld\n" \
            "1:\tlodsl\n\t" \
            "notl %%eax\n\t" \
            "bsfl %%eax, %%edx\n\t" \
            "jne 2f\n\t" \
            "addl $32, %%ecx\n\t" \
            "cmpl $8192,%%ecx\n\t" \
            "jl 1b\n\t" \
            "xorl %%edx,%%edx\n" \
            "2:\taddl %%edx,%%ecx" \
            :"=c" (__res):"0" (0), "S" (addr)); \
    __res;})


extern void minix_free_block(struct super_block * sb, int block);

/*
 * Allocate new block to inode.
 *  On MINIX-FS, a new block is a data zone. In order to allocate new
 *  data zone, we should find a empty zone from Zone-BitMap. 
 *
 *  Note! Zone-BitMap is used to manage all data zone, data zone is belong 
 *  to zone on MINIX-FS. The MINIX-FS divide filesystem into a unit, and 
 *  the size of unit is BLOCK_SIZE. The zone contains Reserved zone and
 *  data zone.
 *
 *  1) Reserved Zone
 *     Reserved zone is used to store MINIX-FS information, such as boot
 *     block, super block, zone-bitmap, inode-bitmap and inode-table.
 *  2) Data Zone
 *     Data zone is used to store data of inode.
 * 
 *  Note! The Zone-Bitmap is used to manage all data zone not contain 
 *  Reserved zone. And 's_firstdatazone' points to first data zone.
 */
static int minix_new_blocks(struct super_block *sb)
{
    struct buffer_head *bh;
    int i, j;

    if (!sb) {
        printk("trying to get new block from nonexistent device\n");
        return 0;
    }
repeat:
    j = 8192;
    for (i = 0; i < 8; i++)
        if ((bh = sb->u.minix_sb.s_zmap[i]) != NULL)
            if ((j = find_first_zero(bh->b_data)) < 8192)
                break;
    if (i >= 8 || !bh || j >= 8192)
        return 0;
    if (set_bit(j, bh->b_data)) {
        printk("new_block: bit already set\n");
        goto repeat;
    }
    bh->b_dirt = 1;
    /*
     * +------+------------+-----+-------------+----------------------+
     * |      |            |     |             |                      |
     * | Boot | Superblock | ... | Inode-Table | Data zone            |
     * |      |            |     |             |                      |
     * +------+------------+-----+-------------+----------------------+
     *                                         A
     *                                         |
     *                                         |
     *                                         |
     *                  s_firstdatazone--------o
     *
     */

    j += i * 8192 + sb->u.minix_sb.s_firstdatazone - 1;
    if (j < sb->u.minix_sb.s_firstdatazone ||
                    j > sb->u.minix_sb.s_nzones)
        return 0;
    if (!(bh = getblk(sb->s_dev, j, BLOCK_SIZE))) {
        printk("new_block: cannot get block\n");
        return 0;
    }
    clear_block(bh->b_data);
    bh->b_uptodate = 1;
    bh->b_dirt = 1;
    brelse(bh);
    return j;
}

/*
 * On MINIX-FS directly access buffer
 *  The inode manages all block nr on 'inode->u.minix_i.i_data[x]'.
 *  and then read data from disk to buffer via 'bread'.
 *
 *  +-----------+
 *  |           |
 *  |   inode   |
 *  |           |                       +-----------------+
 *  +-----------+                       |                 |
 *  | u.minix_i |                       |   buffer_head   |
 *  |           |                       |                 |
 *  | i_data[0]-|-------> block_nr----->+-----------------+
 *  |           |
 *  | ....      |
 *  |           |
 *  |i_data[15] |
 *  +-----------+
 *
 */
static struct buffer_head *inode_getblks(struct inode *inode, int nr, 
                             int create)
{
    int tmp;
    unsigned short *p;
    struct buffer_head *result;

    p = inode->u.minix_i.i_data + nr;
repeat:
    tmp = *p;
    if (tmp) {
        result = getblk(inode->i_dev, tmp, BLOCK_SIZE);
        if (tmp == *p)
            return result;
        brelse(result);
        goto repeat;
    }
    if (!create)
        return NULL;
    tmp = minix_new_blocks(inode->i_sb);
    if (!tmp)
        return NULL;
    result = getblk(inode->i_dev, tmp, BLOCK_SIZE);
    if (*p) {
        minix_free_block(inode->i_sb, tmp);
        brelse(result);
        goto repeat;
    }
    *p = tmp;
    inode->i_ctime = CURRENT_TIME;
    inode->i_dirt = 1;
    return result;
}

/*
 * block_getblks
 *  Obtain block nr from a block. For this way, MINIX-FS will one or two
 *  indirectly access special block nr, as figure:
 *
 * One indirectly access
 *
 *
 *                                                  +-------------+
 *                                                  |             |
 * +-------------+                block_array       | buffer_head |
 * |             |              +-------------+     |             |
 * | minix_inode |              | block_order-|---->+-------------+
 * |             |              +-------------+
 * +-------------+              | ..........  |
 * |             |              +-------------+
 * +-------------+  block_order | block_order |
 * |  i_zone[7] -|------------->+-------------+
 * +-------------+
 * |             |
 * +-------------+
 *
 * Twice indirectly access
 *
 *
 *
 *
 *                                          block_array
 *                                        +-------------+
 *                                        | block_order |
 *                                        +-------------+
 *                                        |   ....      |
 * +-------------+      block_array       +-------------+
 * |             |    +-------------+     | block_order-|---------o
 * | minix_inode |    | block_order-|---->+-------------+         |
 * |             |    +-------------+                             |
 * +-------------+    | ..........  |                             |
 * |             |    +-------------+                             |
 * +-------------+    | block_order |                             |
 * |  i_zone[8] -|--->+-------------+                             |
 * +-------------+                        +-------------+<--------o
 *                                        |             |
 *                                        | buffer_head |
 *                                        |             |    
 *                                        +-------------+
 *          
 *   
 * 
 */
static struct buffer_head *block_getblks(struct inode *inode,
       struct buffer_head *bh, int nr, int create)
{
    int tmp;
    unsigned short *p;
    struct buffer_head *result;

    if (!bh)
        return NULL;
    if (!bh->b_uptodate) {
        ll_rw_block(READ, 1, &bh);
        wait_on_buffer(bh);
        if (!bh->b_uptodate) {
            brelse(bh);
            return NULL;
        }
    }
    p = nr + (unsigned short *)bh->b_data;
repeat:
    tmp = *p;
    if (tmp) {
        result = getblk(bh->b_dev, tmp, BLOCK_SIZE);
        if (tmp == *p) {
            brelse(bh);
            return result;
        }
        brelse(result);
        goto repeat;
    }
    if (!create) {
        brelse(bh);
        return NULL;
    }
    tmp = minix_new_blocks(inode->i_sb);
    if (!tmp) {
        brelse(bh);
        return NULL;
    }
    result = getblk(bh->b_dev, tmp, BLOCK_SIZE);
    if (*p) {
        minix_free_block(inode->i_sb, tmp);
        brelse(result);
        goto repeat;
    }
    *p = tmp;
    bh->b_dirt = 1;
    brelse(bh);
    return result;
}

/*
 * Read Inode buffer
 *  On MINIX-FS, each inode manage a series of data zones. Minix-inode
 *  contains 3 type data zone.
 *   
 *    1) Directly access
 *       The block nr is smaller then 7, inode can access buffer directly.
 *
 *    2) One indirect access
 *       The block nr is 7 to 512, and inode should access buffer indirectly 
 *       by once.
 *
 *    3) Two indirect access
 *       The block nr is big then 512, and inode should access buffer 
 *       indirectly by twice.
 * 
 *
 *  minix_inode
 *  
 *  +-------------+
 *  |             |
 *  | Other parts |       +-------------+
 *  |             |       |             |
 *  +-------------+       | buffer_head |
 *  |  i_zone[0] -|------>|             |
 *  +-------------+       +-------------+
 *  |  i_zone[1]  |
 *  +-------------+                           +-------------+
 *  |  i_zone[2]  |                           |             |
 *  +-------------+                           | buffer_head |
 *  |  i_zone[3]  |       +----------+        |             |
 *  +-------------+       |  block0 -|------->+-------------+
 *  |  i_zone[4]  |       +----------+
 *  +-------------+       |  block1  |
 *  |  i_zone[5]  |       +----------+
 *  +-------------+       |  ......  |
 *  |  i_zone[6]  |       +----------+
 *  +-------------+       |  blockn  |        +-------------+
 *  |  i_zone[7] -|------>+----------+        |             |
 *  +-------------+                           | buffer_head |
 *  |  i_zone[8] -|---o                       |             |
 *  +-------------+   |                       +-------------+
 *                    |                                     A
 *                    |                                     |
 *                    |                                     |
 *                    |                     +----------+    |
 *                    |                     |  block0 -|----o
 *                    |                     +----------+
 *                    |                     |  block1  |
 *                    |                     +----------+
 *                    |                     |  ......  |
 *                    |                     +----------+
 *                    o-->+----------+      |  blockn  |
 *                        |  block0 -|----->+----------+
 *                        +----------+
 *                        |  block1  |
 *                        +----------+
 *                        |  ......  |
 *                        +----------+
 *                        |  blockn  |
 *                        +----------+
 *
 */
static struct buffer_head *minix_getblks(struct inode *inode, int block,
                    int create)
{
    struct buffer_head *bh;

    if (block < 0) {
        printk("minix_getblk: block < 0");
        return NULL;
    }
    if (block >= 7 + 512 + 512 * 512) {
        printk("minix_getblk: block > big");
        return NULL;
    }
    if (block < 7)
        return inode_getblks(inode, block, create);
    block -= 7;
    if (block < 512) {
        bh = inode_getblks(inode, 7, create);
        return block_getblks(inode, bh, block, create);
    }
    block -= 512;
    bh = inode_getblks(inode, 8, create);
    bh = block_getblks(inode, bh, block >> 9, create);
    return block_getblks(inode, bh, block & 511, create);
}

int minix_file_write(struct inode *inode, struct file *filp,
          char *buf, int count)
{
    off_t pos;
    int written, c;
    struct buffer_head *bh;
    char *p;

    if (!inode) {
        printk("minix_file_write: inode = NULL\n");
        return -EINVAL;
    }
    if (!S_ISREG(inode->i_mode)) {
        printk("minix_file_write: mode = %07o\n", inode->i_mode);
        return -EINVAL;
    }
    /*
     * OK, append may not work when many processes are writing at the
     * same time but so what. That way leads to madness anyway.
     *
     *
     * +-----------+
     * |           |       +----------------------------------+
     * |   inode   |       |                                  |
     * |           |       |            buffer_head           |
     * |           |       |                                  |
     * | i_data[0]-|------>+----------------------------------+
     * |           |                                       |
     * +-----------+                                       |
     *                                                     | b_data
     *                                                     |
     * 1) f_flags: ~O_APPEND                               |
     * +--------------+--------------------------------+<--o
     * |              |                                |   |
     * |              |                                |   |
     * |              |                                |   |
     * +--------------+--------------------------------+   |
     *                A                                    |
     *                |                                    |
     *                |                                    ~
     * filp->f_pos----o                                    |
     *                                                     | 
     * pos = filp->f_pos                                   |
     *                                                     |
     *                                                     |
     * 2) f_flags: O_APPEND                                |
     * +--------------+--------------------------------+<--o
     * |              |                                | 
     * |              |                                | 
     * |              |                                | 
     * +--------------+--------------------------------+
     *                A                                A
     *                |                                |
     *                |                                |
     * filp->f_pos----o                      pos-------o
     *
     * pos = inode->i_size
     *
     */
    if (filp->f_flags & O_APPEND)
        pos = inode->i_size;
    else
        pos = filp->f_pos;
    written = 0;
    while (written < count) {
        /* Read buffer from file */
        bh = minix_getblks(inode, pos / BLOCK_SIZE, 1);
        if (!bh) {
            if (!written)
                written = -ENOSPC;
            break;
        }
        /* 'c' indicate the count for read on current BLOCK */
        c = BLOCK_SIZE - (pos % BLOCK_SIZE);
        if (c > count - written)
            c = count - written;
        if (c != BLOCK_SIZE && !bh->b_uptodate) {
            ll_rw_block(READ, 1, &bh);
            wait_on_buffer(bh);
            if (!bh->b_uptodate) {
                brelse(bh);
                if (!written)
                    written = -EIO;
                break;
            }
        }
        /* Point to locate for read on current block */
        p = (pos % BLOCK_SIZE) + bh->b_data;
        pos += c;
        if (pos > inode->i_size) {
            inode->i_size = pos;
            inode->i_dirt = 1;
        }
        written += c;
        memcpy_fromfs(p, buf, c);
        buf += c;
        bh->b_uptodate = 1;
        bh->b_dirt = 1;
        brelse(bh);
    }
    inode->i_mtime = inode->i_ctime = CURRENT_TIME;
    filp->f_pos = pos;
    inode->i_dirt = 1;
    return written;
}

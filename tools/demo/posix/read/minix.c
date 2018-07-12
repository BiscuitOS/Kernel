/*
 * POSIX read on MINIX-FS
 *
 * (C) 2018.07.11 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/stat.h>
#include <linux/locks.h>

#include <asm/segment.h>

#include <demo/debug.h>

#define NBUF      32

extern void minix_free_block(struct super_block * sb, int block);
extern int minix_new_block(struct super_block * sb);

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
        /* get a buffer_head, more see tools/demo/vfs/core/buffer/buffer.c */
        result = getblk(inode->i_dev, tmp, BLOCK_SIZE);
        if (tmp == *p)
            return result;
        brelse(result);
        goto repeat;
    }
    if (!create)
        return NULL;
    tmp = minix_new_block(inode->i_sb);
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
    p = nr + (unsigned short *) bh->b_data;
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
struct buffer_head *minix_getblks(struct inode *inode, int block,
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

/*
 * file_read_minix()
 *  Read data from minixfs. This function utilize two 'buffer_head' to
 *  manage special 'buffer_head'. "bhreq" is used to manage 'buffer_head'
 *  that contain data for reading and 'bhreq' will be invoke when reading
 *  data from disk. "bhlist" is also manage 'buffer_head' and 'bhb' points
 *  to last 'buffer_head' on array, 'bhe' points to first 'buffer_head' on
 *  'bhlist'. The 'bhlist' is used to count read and exchange data into 
 *  userland.
 */
int file_read_minix(struct inode *inode, struct file *filp,
           char *buf, int count)
{
    int read, left, chars;
    int block, blocks, offset;
    int bhrequest, uptodate;
    struct buffer_head **bhb, **bhe;
    struct buffer_head *bhreq[NBUF];
    struct buffer_head *buflist[NBUF];
    unsigned int size;

    if (!inode) {
        printk(KERN_ERR "minix_file_read: inode = NULL\n");
        return -EINVAL;
    }
    if (!S_ISREG(inode->i_mode)) {
        printk(KERN_ERR "minix_file_read: mode = %07o\n", inode->i_mode);
        return -EINVAL;
    }
    /*
     * Verify range for read.
     *  On reading file, subsystem utilize 'size', 'count', 'left' and 
     *  'offset' to indicate block and block number for reading.
     *
     *  The 'size' indicate the lenght of file.
     *  The 'offset' indicate the start address for reading.
     *  The 'count' indicate the number for reading.
     *  The 'left' indicate the number for remain data.
     *
     *  Case 1:
     *   "offset < size" and "count < left"
     *
     *   | <----------------------- size -------------------------> |
     *   +------------+--------------------+------------------------+
     *   |            |                    |                        |
     *   |            | <--- Data area --> |                        |
     *   |            |                    |                        |
     *   +------------+--------------------+------------------------+
     *                A <-------------------- left ---------------> |
     *                | <---- count -----> |
     *                |
     *                |
     *      offset----o 
     *
     *      (1) left = size - offset
     *      (2) left = count
     *
     *  Case 2:
     *   "offset > size"
     *
     *   | <---------------- size --------------> |
     *   +------------+--------------------+------+--+-------------+
     *   |            |                    |      |  |             |
     *   |            | <--- Data area --> |      |  |             |
     *   |            |                    |      |  |             |
     *   +------------+--------------------+------+--+-------------+
     *                                               A <- count -> |
     *                                               |
     *                                               |
     *                                     offset----o 
     *
     *      (1) left = 0
     *      (2) BAD && reutrn 0
     *
     *
     *
     *  Case 3:
     *   "offset < size" and "count > left"
     *
     *   | <---------------- size --------------> |
     *   +------------+--------------------+------+-----------------+
     *   |            |                    |      |                 |
     *   |            | <--- Data area --> |      |                 |
     *   |            |                    |      |                 |
     *   +------------+--------------------+------+-----------------+
     *                A <------------------- count ---------------> |
     *                | <--------- left --------> |
     *                |
     *                |
     *      offset----o 
     *
     *
     *      (1) left = size - left
     *      (2) left < count
     */
    offset = filp->f_pos;
    size = inode->i_size;
    if (offset > size)
        left = 0;
    else
        left = size - offset;
    if (left > count)
        left = count;
    if (left <= 0)
        return 0;
    read = 0;
    /*
     * The inode contains a lot of blocks, and the 'block' argument indicates
     * offset on block set. The 'blocks' indicates last block offset.
     *
     *
     *
     * Inode block set
     * +--------+--------+-----+--------+-----+----------+--------+
     * |        |        |     |        |     |          |        |
     * | BLOCK0 | BLOCK1 | ... | BLOCKm | ... | BLOCKm+n | BLOCKx |
     * |        |        |     |        |     |          |        |
     * +--------+--------+-----+--------+-----+----------+--------+
     *                         A              A
     *                         |              |
     *                         |              |
     *                 block---o              |
     *                                        |
     *                                        |
     *                 blocks-----------------o
     *
     */
    block = offset >> BLOCK_SIZE_BITS;
    offset &= BLOCK_SIZE - 1;
    size = (size + (BLOCK_SIZE - 1)) >> BLOCK_SIZE_BITS;
    blocks = (left + offset + BLOCK_SIZE - 1) >> BLOCK_SIZE_BITS;
    /*
     * The 'buflist' is an array that holds all request 'buffer_head'. 
     * 'bhe' points to the first 'buffer_head' and 'bhb' points to the
     * last 'buffer_head'. As figure:
     *
     *
     * buflist[NBUF]:
     *
     * +-------------+-------------+----------+-------------+------+
     * |             |             |          |             |      |
     * | Buffer_head | Buffer_head | ...      | Buffer_head | ...  |
     * |             |             |          |             |      |
     * +-------------+-------------+----------+-------------+------+
     * A                                      
     * |                                      
     * |                                      
     * o----bhe/bhb
     *
     *
     */
    bhb = bhe = buflist;
    if (filp->f_reada) {
        blocks += read_ahead[MAJOR(inode->i_dev)] / (BLOCK_SIZE >> 9);
        if (block + blocks > size)
            blocks = size - block;
    }

    /* We do this in a two stage process. We first try and request
     * as many blocks as we can, then we wait for the first one to
     * complete, and then we try and wrap up as many as are actually
     * done. This routine is rather generic, in that it can be used
     * in a filesystem by substituting the appropriate function in
     * for getblk.
     *
     * This routine is optimized to make maximum use of the various
     * buffers and caches. */

    do {
        bhrequest = 0;
        uptodate = 1;
        while (blocks) {
            --blocks;
            *bhb = minix_getblks(inode, block++, 0);
            /*
             * The system invokes 'minix_getblks()' to get speical 
             * 'buffer_head', and if b_uptodate on buffer_head that indicate
             *  buffer need reload, and add buffer into request queue.
             *
             * bhreq[NBUF]
             * 
             * +-------------+-------------+--------------+-------------+
             * |             |             |              |             |
             * | buffer_head | buffer_head | ...          | buffer_head |
             * |             |             |              |             |
             * +-------------+-------------+--------------+-------------+
             *
             */
            if (*bhb && !(*bhb)->b_uptodate) {
                uptodate = 0;
                bhreq[bhrequest++] = *bhb;
            }

            /* Uptodate 'bhb' and point next bufferlist. If 'bhb' points
             * to end of 'buflist' and then make 'bhb' points to header 
             * of 'buflist' */
            if (++bhb == &buflist[NBUF])
                bhb = buflist;

            /* If the block we have on hand is uptodate, go ahead 
               and complete processing. */
            if (uptodate)
                break;
            if (bhb == bhe)
                break;
        }

        /* Now request them all */
        if (bhrequest)
            ll_rw_block(READ, bhrequest, bhreq);

        do { /* 
              * Finish off all I/O that has actually completed
              *
              * buflist[NBUF]:
              *
              * +-------------+-------------+----------+-------------+------+
              * |             |             |          |             |      |
              * | Buffer_head | Buffer_head | ...      | Buffer_head | ...  |
              * |             |             |          |             |      |
              * +-------------+-------------+----------+-------------+------+
              * A                                      A
              * |                                      |
              * |                                      |
              * o----bhe                          bhb--o
              *
    printk("Value %s\n", value);
              */ 

            if (*bhe) {
                wait_on_buffer(*bhe);
                if (!(*bhe)->b_uptodate) {    /* read error? */
                    brelse(*bhe);
                    if (++bhe == &buflist[NBUF])
                        bhe = buflist;
                    left = 0;
                    break;
                }
            }
            /*
             * The 'left' indicate number for reading. If number is small
             * then "BLOCK_SIZE - offset", 'chars' is equal 'left'
             *
             *
             * | <---------- BLOCK SIZE ------------> |
             * 0---------------+-------------------+--1K
             * |               |                   |  |
             * |               |                   |  |
             * |               |                   |  |
             * +---------------+-------------------+--+
             *                 A <---- left -----> |
             *                 | <---- chars ----> |
             *   offset -------o
             *
             * (1) chars = left
             *
             *
             * If number for reading is big than remain on a BLOCK_SIZE.
             * and the 'chars' is equal remain number.
             *
             * | <---------- BLOCK SIZE ------------> |
             * 0---------------+----------------------1K
             * |               |                      |
             * |               |                      |
             * |               |                      |
             * +---------------+----------------------+
             *                 A <------------- left ----------> |
             *                 | <------ chars -----> |
             *   offset -------o
             *
             * (1) chars = BLOCK_SIZE - offset
             *
             */
            if (left < BLOCK_SIZE - offset)
                chars = left;
            else
                chars = BLOCK_SIZE - offset;
            /* update argument for next reading */
            filp->f_pos += chars;
            left -= chars;
            read += chars;
            if (*bhe) { /* Copy data from buffer to userland */
                memcpy_tofs(buf, offset + (*bhe)->b_data, chars);
                brelse(*bhe);
                buf += chars;
            } else {
                while (chars-- > 0)
                    put_fs_byte(0, buf++);
            }
            offset = 0;
            /* Uptodate 'bhe' and verify whether 'bhe' points to end of 
             * 'buflist'. If it is done and makes 'bhe' points to start
             * of 'buflist' */
            if (++bhe == &buflist[NBUF])
                bhe = buflist;
        } while (left > 0 && bhe != bhb && (!*bhe || !(*bhe)->b_lock));
    } while (left > 0);

    /* Release the read-ahead blocks */
    while (bhe != bhb) {
        brelse(*bhe);
        if (++bhe == &buflist[NBUF])
            bhe = buflist;
    }
    if (!read)
        return -EIO;
    filp->f_reada = 1;
    if (IS_RDONLY(inode))
        inode->i_atime = CURRENT_TIME;
    return read;
}

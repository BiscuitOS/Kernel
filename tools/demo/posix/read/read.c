/*
 * POSIX system call: read
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
#include <linux/mm.h>
#include <linux/stat.h>
#include <linux/locks.h>

#include <asm/segment.h>

#include <demo/debug.h>

#define NBUF      32

static inline _syscall3(int, open, const char *, file, int, flag, int, mode);
static inline _syscall1(int, close, int, fd);

#ifdef CONFIG_MINIX_FS

static struct buffer_head *inode_getblk(struct inode *inode, int nr,
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
        return 0;
    return NULL;
}

struct buffer_head *minix_getblks(struct inode *inode, int block,
                    int create)
{
    if (block < 0) {
        printk("minix_getblk: block < 0");
        return NULL;
    }
    if (block >= 7 + 512 + 512 * 512) {
        printk("minix_getblk: block > big");
        return NULL;
    }
    if (block < 7)
        return inode_getblk(inode, block, create);
    panic("MINIX_GETBLOPCK");
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
static int file_read_minix(struct inode *inode, struct file *filp,
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
     * A                                      A
     * |                                      |
     * |                                      |
     * o----bhe                       bhb-----o
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
             * 'buffer_head', and then 'bhreq' array hold 'buffer_head'.
             * 'bhreq' array is used to read data from disk.
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

        do { /* Finish off all I/O that has actually completed */
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
#endif

#ifdef CONFIG_DEBUG_READ_ROOTFS
static int rootfs_read(struct inode *inode, struct file *filp, char *buf,
                     unsigned int count)
{
#ifdef CONFIG_MINIX_FS
    file_read_minix(inode, filp, buf, count);
#endif
    return 0;
}
#endif

/* This may be used only once, enforced by 'static int callable' */
asmlinkage int sys_demo_read(unsigned int fd, char *buf, unsigned int count)
{
    int error;
    struct file *file;
    struct inode *inode;

    /*
     * Each task structure manage all opened file descriptor on filp,
     * we can get special file structure via fd. Unique inode structure
     * hold on file structure that points to special inode. NOTE!
     * each task only can manage 'NR_OPEN' file structure.
     */
    if (fd >= NR_OPEN || !(file = current->filp[fd]) ||
                         !(inode = file->f_inode))
        return -EBADF;
    /*
     * Note that while the flag value (low two bits) for sys_open means:
     *     00 - read-only
     *     01 - write-only
     *     10 - read-write
     *     11 - special
     * On file descriptor it is changed into 
     *     00 - no permission needed
     *     01 - read-permission
     *     10 - write-permission
     *     11 - read-write
     * So on sys_read, f_mode must need read-permission..
     */
    if (!(file->f_mode & 1))
        return -EBADF;
    if (!file->f_op || !file->f_op->read)
        return -EINVAL;
    if (!count)
        return 0;
    /*
     * Verify whether userland address is safe that doesn't over TASK_SIZE.
     */
    error = verify_area(VERIFY_WRITE, buf, count);
    if (error)
        return error;
#ifdef CONFIG_DEBUG_READ_ROOTFS
    return rootfs_read(inode, file, buf, count);
#else
    return file->f_op->read(inode, file, buf, count);
#endif
}

/* build a system call entry for read */
inline _syscall3(int, demo_read, unsigned int, fd, char *, buf,
                                         unsigned int, count);

/* userland code */
static int debug_read(void)
{
    int fd;
    char buf[20];

    fd = open("/etc/rc", O_RDONLY, 0);
    if (fd < 0) {
        printf("Unable open '/etc/rc'\n");
        return -1;
    }

    demo_read(fd, buf, 10);
    printf("Read: %s\n", buf);
    close(fd);
    return 0;
}
user1_debugcall_sync(debug_read);

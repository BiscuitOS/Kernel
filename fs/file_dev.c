/*
 * linux/fs/file_dev.c
 *
 * (C) 1991 Linus Torvalds
 */
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/sched.h>

#include <asm/segment.h>

#include <errno.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

int file_read(struct m_inode *inode, struct file *filp, char *buf, int count)
{
    int left, chars, nr;
    struct buffer_head *bh;

    if ((left = count) <= 0)
        return 0;
    while (left) {
        if ((nr = bmap(inode, (filp->f_pos) / BLOCK_SIZE))) {
            if (!(bh = bread(inode->i_dev, nr)))
                break;
        } else
            bh = NULL;
        nr = filp->f_pos % BLOCK_SIZE;
        chars = MIN(BLOCK_SIZE - nr, left);
        filp->f_pos += chars;
        left -= chars;
        if (bh) {
            char *p = nr + bh->b_data;

            while (chars-- > 0)
                put_fs_byte(*(p++), buf++);
            brelse(bh);
        } else {
            while (chars-- > 0)
                put_fs_byte(0, buf++);
        }
    }
    inode->i_atime = CURRENT_TIME;
    return (count - left) ? (count - left) : -ERROR;
}

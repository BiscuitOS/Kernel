/*
 * linux/fs/block_dev.c
 *
 * (C) 1991 Linus Torvalds
 */

#include <linux/fs.h>

#include <asm/segment.h>

#include <errno.h>

int block_read(int dev, unsigned long *pos, char *buf, int count)
{
    int block = *pos >> BLOCK_SIZE_BITS;
    int offset = *pos & (BLOCK_SIZE - 1);
    int chars;
    int read = 0;
    struct buffer_head *bh;
    register char *p;

    while (count > 0) {
        chars = BLOCK_SIZE - offset;
        if (chars > count)
            chars = count;
        if (!(bh = breada(dev, block, block + 1, block + 2, -1)))
            return read ? read : -EIO;
        block++;
        p = offset + bh->b_data;
        offset = 0;
        *pos += chars;
        read += chars;
        count -= chars;
        while (chars-- > 0)
            put_fs_byte(*(p++), buf++);
        brelse(bh);
    }
    return read;
}

/*
 * linux/fs/pipe.c
 *
 * (C) 1991 Linus Torvalds
 */
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <asm/segment.h>

int read_pipe(struct m_inode *inode, char *buf, int count)
{
    int chars, size, read = 0;

    while (count > 0) {
        while (!(size = PIPE_SIZE(*inode))) {
            wake_up(&inode->i_wait);
            if (inode->i_count != 2) /* are there any writers? */
                return read;
            sleep_on(&inode->i_wait);
        }
        chars = PAGE_SIZE - PIPE_TAIL(*inode);
        if (chars > count)
            chars = count;
        if (chars > size)
            chars = size;
        count -= chars;
        read  += chars;
        size = PIPE_TAIL(*inode);
        PIPE_TAIL(*inode) += chars;
        PIPE_TAIL(*inode) &= (PAGE_SIZE - 1);
        while (chars-- > 0)
            put_fs_byte(((char *)inode->i_size)[size++], buf++);
    }
    wake_up(&inode->i_wait);
    return read;
}

int write_pipe(struct m_inode *inode, char *buf, int count)
{
    int chars, size, written = 0;

    while (count > 0) {
        while (!(size = (PAGE_SIZE - 1) - PIPE_SIZE(*inode))) {
            wake_up(&inode->i_wait);
            if (inode->i_count != 2) { /* no readers */
                current->signal |= (1 << (SIGPIPE - 1));
                return written ? written : -1;
            }
            sleep_on(&inode->i_wait);
        }
        chars = PAGE_SIZE - PIPE_HEAD(*inode);
        if (chars > count)
            chars = count;
        if (chars > size)
            chars = size;
        count -= chars;
        written += chars;
        size = PIPE_HEAD(*inode);
        PIPE_HEAD(*inode) += chars;
        PIPE_HEAD(*inode) &= (PAGE_SIZE - 1);
        while (chars-- > 0)
            ((char *)inode->i_size)[size++] = get_fs_byte(buf++);
    }
    wake_up(&inode->i_wait);
    return written;
}
